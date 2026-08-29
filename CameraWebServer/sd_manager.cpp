// ============================================================
//  sd_manager.cpp  –  SD card + AVI video recording
//  SD_MMC 1-bit mode (GPIO 2=D0, 14=CLK, 15=CMD)
//  GPIO 4 is the flash LED — MUST NOT be used by SD_MMC
// ============================================================
#include "sd_manager.h"
#include <SD_MMC.h>
#include <FS.h>
#include <time.h>
#include "esp_camera.h"
#include <freertos/semphr.h>
#include "telegram_worker.h"
#include <Preferences.h>

extern SemaphoreHandle_t camera_mutex;
extern Preferences       preferences;

static bool  g_sd_mounted = false;
SemaphoreHandle_t g_sd_mutex = nullptr;

// ─── AVI file helpers ────────────────────────────────────────
// Minimal RIFF-AVI container for Motion-JPEG
// Reference: OpenDML AVI spec (simplified)
#define FOUR_CC(a,b,c,d) ((uint32_t)(a)|((uint32_t)(b)<<8)|((uint32_t)(c)<<16)|((uint32_t)(d)<<24))

static void writeU32LE(File& f, uint32_t v) {
    f.write((uint8_t*)&v, 4);
}
static void writeFCC(File& f, const char* cc) {
    f.write((uint8_t*)cc, 4);
}
static void writeSdChunked(File& f, const uint8_t* buf, size_t len) {
    if (!buf || len == 0) return;
    size_t written = 0;
    while (written < len) {
        size_t toWrite = min((size_t)2048, len - written);
        f.write(buf + written, toWrite);
        written += toWrite;
    }
}

// Patch a 32-bit field at given file offset (then seek back to end)
static void patchU32(File& f, uint32_t pos, uint32_t val) {
    uint32_t cur = f.position();
    f.seek(pos);
    writeU32LE(f, val);
    f.seek(cur);
}

// ─── Mount ───────────────────────────────────────────────────
bool sd_manager_init() {
    g_sd_mutex = xSemaphoreCreateMutex();

    // GPIO 4 is Flash LED — release it before SD_MMC can claim it
    // In 1-bit mode D0=GPIO2, CLK=GPIO14, CMD=GPIO15. GPIO4 is NOT used.
    pinMode(4, OUTPUT);   // keep it as output so we can still toggle flash
    digitalWrite(4, LOW);

    if (!SD_MMC.begin("/sdcard", true)) {  // true = 1-bit mode
        Serial.println("[SD] Mount FAILED");
        telegram_send_message("⚠️ SD Card mount failed!");
        return false;
    }
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[SD] No card found");
        telegram_send_message("⚠️ No SD card detected!");
        SD_MMC.end();
        return false;
    }
    g_sd_mounted = true;

    // Ensure directories exist
    SD_MMC.mkdir("/photos");
    SD_MMC.mkdir("/videos");

    uint64_t total = SD_MMC.totalBytes();
    uint64_t used  = SD_MMC.usedBytes();
    char buf[180];
    snprintf(buf, sizeof(buf),
        "💾 SD Card mounted (%s)\nTotal: %lluMB | Used: %lluMB | Free: %lluMB",
        cardType == CARD_SD ? "SD" : "SDHC",
        total / (1024*1024), used / (1024*1024), (total-used) / (1024*1024)
    );
    telegram_send_message(buf);
    Serial.printf("[SD] Mounted. Total=%lluMB Free=%lluMB\n",
        total/(1024*1024), (total-used)/(1024*1024));
    return true;
}

bool sd_is_mounted() { return g_sd_mounted; }

void sd_get_info(uint64_t& total_bytes, uint64_t& used_bytes) {
    if (!g_sd_mounted) { total_bytes = used_bytes = 0; return; }
    total_bytes = SD_MMC.totalBytes();
    used_bytes  = SD_MMC.usedBytes();
}

// ─── Timestamp helpers ────────────────────────────────────────
static String getTimestamp() {
    struct tm ti;
    if (!getLocalTime(&ti, 1000)) {
        // fallback: use millis
        char buf[20];
        uint32_t ms = millis();
        snprintf(buf, sizeof(buf), "%010lu", ms);
        return String(buf);
    }
    char buf[20];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &ti);
    return String(buf);
}

// ─── Save photo ───────────────────────────────────────────────
void sd_save_photo(uint8_t* buf, size_t len) {
    if (!g_sd_mounted || !buf || len == 0) return;
    if (xSemaphoreTake(g_sd_mutex, pdMS_TO_TICKS(2000)) != pdTRUE) return;
    String path = "/photos/PHOTO_" + getTimestamp() + ".jpg";
    File f = SD_MMC.open(path.c_str(), FILE_WRITE);
    if (f) {
        writeSdChunked(f, buf, len);
        f.close();
        Serial.printf("[SD] Photo saved: %s (%u bytes)\n", path.c_str(), len);
    }
    xSemaphoreGive(g_sd_mutex);
}

// ─── Auto-delete oldest video when space is low ──────────────
static void autoCleanVideos() {
    uint64_t free_bytes = SD_MMC.totalBytes() - SD_MMC.usedBytes();
    File dir = SD_MMC.open("/videos");
    if (!dir || !dir.isDirectory()) return;

    // Build sorted list by name (ISO names sort chronologically)
    std::vector<String> names;
    File entry = dir.openNextFile();
    while (entry) {
        if (!entry.isDirectory()) names.push_back(String(entry.name()));
        entry = dir.openNextFile();
    }
    dir.close();
    std::sort(names.begin(), names.end());

    // Delete oldest if free < 50MB or > 20 files
    while ((free_bytes < 50ULL * 1024 * 1024 || names.size() > 20) && !names.empty()) {
        String path = "/videos/" + names.front();
        Serial.printf("[SD] Auto-delete: %s\n", path.c_str());
        SD_MMC.remove(path.c_str());
        names.erase(names.begin());
        free_bytes = SD_MMC.totalBytes() - SD_MMC.usedBytes();
    }
}

// ─── Direct-to-SD AVI Stream Recorder ────────────────────────
struct AviIndexEntry {
    uint32_t offset;
    uint32_t size;
};

void TaskRecording(void* pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(5000));  // let camera and wifi settle

    for (;;) {
        bool enabled = preferences.getBool("rec_enabled", true);
        uint32_t interval_min = preferences.getUInt("rec_interval", 15);
        if (!enabled || !g_sd_mounted) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        if (interval_min < 1) interval_min = 1;
        if (interval_min > 60) interval_min = 60;

        uint32_t fps = 1;                     // 1 frame per second for smooth background time-lapse
        uint32_t frame_interval_ms = 1000 / fps;
        uint32_t target_duration_ms = interval_min * 60 * 1000UL;

        // Check SD free space before starting new recording
        uint64_t free_bytes = SD_MMC.totalBytes() - SD_MMC.usedBytes();
        if (free_bytes < 30ULL * 1024 * 1024) {
            autoCleanVideos();
            free_bytes = SD_MMC.totalBytes() - SD_MMC.usedBytes();
            if (free_bytes < 30ULL * 1024 * 1024) {
                Serial.println("[REC] SD space low, skipping recording");
                vTaskDelay(pdMS_TO_TICKS(30000));
                continue;
            }
        }

        if (xSemaphoreTake(g_sd_mutex, pdMS_TO_TICKS(2000)) != pdTRUE) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        String path = "/videos/VID_" + getTimestamp() + ".avi";
        File f = SD_MMC.open(path.c_str(), FILE_WRITE);
        if (!f) {
            xSemaphoreGive(g_sd_mutex);
            Serial.printf("[REC] Failed to create %s\n", path.c_str());
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        Serial.printf("[REC] Recording started: %s (%u min @ %u fps)\n", path.c_str(), interval_min, fps);

        // Get initial camera resolution
        sensor_t* s = esp_camera_sensor_get();
        uint32_t width = 640, height = 480;
        if (s) {
            switch (s->status.framesize) {
                case FRAMESIZE_UXGA: width = 1600; height = 1200; break;
                case FRAMESIZE_SXGA: width = 1280; height = 1024; break;
                case FRAMESIZE_XGA:  width = 1024; height = 768;  break;
                case FRAMESIZE_SVGA: width = 800;  height = 600;  break;
                case FRAMESIZE_VGA:  width = 640;  height = 480;  break;
                case FRAMESIZE_CIF:  width = 400;  height = 296;  break;
                case FRAMESIZE_QVGA: width = 320;  height = 240;  break;
                default:             width = 640;  height = 480;  break;
            }
        }

        // ── 1. RIFF Header ──
        writeFCC(f, "RIFF");
        uint32_t riffSzPos = f.position();
        writeU32LE(f, 0);          // placeholder for RIFF size
        writeFCC(f, "AVI ");

        // ── 2. LIST hdrl ──
        writeFCC(f, "LIST");
        writeU32LE(f, 4 + 56 + 4 + 4 + 40 + 4 + 40); // hdrl size
        writeFCC(f, "hdrl");

        // avih
        writeFCC(f, "avih");
        writeU32LE(f, 56);
        writeU32LE(f, 1000000 / fps);     // microseconds per frame
        writeU32LE(f, 0);                  // max bytes per sec
        writeU32LE(f, 0);                  // padding
        writeU32LE(f, 0x10);               // flags: AVIF_HASINDEX
        uint32_t avihFramesPos = f.position();
        writeU32LE(f, 0);                  // placeholder for total frames
        writeU32LE(f, 0);                  // initial frames
        writeU32LE(f, 1);                  // streams
        writeU32LE(f, 0);                  // suggested buffer size
        writeU32LE(f, width);
        writeU32LE(f, height);
        writeU32LE(f, 0); writeU32LE(f, 0); writeU32LE(f, 0); writeU32LE(f, 0); // reserved

        // LIST strl
        writeFCC(f, "LIST");
        writeU32LE(f, 4 + 4 + 4 + 40 + 4 + 4 + 40);
        writeFCC(f, "strl");

        // strh
        writeFCC(f, "strh");
        writeU32LE(f, 40);
        writeFCC(f, "vids");
        writeFCC(f, "MJPG");
        writeU32LE(f, 0);           // flags
        writeU32LE(f, 0);           // priority
        writeU32LE(f, 0);           // initial frames
        writeU32LE(f, 1);           // scale
        writeU32LE(f, fps);         // rate
        writeU32LE(f, 0);           // start
        uint32_t strhFramesPos = f.position();
        writeU32LE(f, 0);           // placeholder for length (frames)
        writeU32LE(f, 0);           // suggested buffer
        writeU32LE(f, -1);          // quality
        writeU32LE(f, 0);           // sample size
        writeU32LE(f, 0); writeU32LE(f, 0); // rcFrame

        // strf (BITMAPINFOHEADER)
        writeFCC(f, "strf");
        writeU32LE(f, 40);
        writeU32LE(f, 40);          // biSize
        writeU32LE(f, width);
        writeU32LE(f, height);
        f.write((uint8_t[]){1, 0}, 2);  // planes=1
        f.write((uint8_t[]){24, 0}, 2); // bit count=24
        writeFCC(f, "MJPG");        // compression
        writeU32LE(f, width * height * 3); // sizeImage
        writeU32LE(f, 0); writeU32LE(f, 0); writeU32LE(f, 0); writeU32LE(f, 0);

        // ── 3. LIST movi ──
        writeFCC(f, "LIST");
        uint32_t moviSzPos = f.position();
        writeU32LE(f, 0);           // placeholder for movi size
        writeFCC(f, "movi");

        uint32_t moviStart = f.position() - 4; // reference for index offsets
        std::vector<AviIndexEntry> index_entries;
        index_entries.reserve(fps * interval_min * 60 + 50);

        uint32_t start_ms = millis();
        uint32_t frame_count = 0;

        while (millis() - start_ms < target_duration_ms) {
            if (!preferences.getBool("rec_enabled", true)) break;

            uint32_t loop_start = millis();

            uint8_t* frame_copy = nullptr;
            size_t   frame_copy_len = 0;

            if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                camera_fb_t* fb = esp_camera_fb_get();
                if (fb && fb->format == PIXFORMAT_JPEG && fb->len > 0) {
                    frame_copy = (uint8_t*)malloc(fb->len);
                    if (frame_copy) {
                        frame_copy_len = fb->len;
                        memcpy(frame_copy, fb->buf, frame_copy_len);
                    }
                }
                if (fb) esp_camera_fb_return(fb);
                xSemaphoreGive(camera_mutex); // Release mutex immediately in < 1ms!
            }

            if (frame_copy && frame_copy_len > 0) {
                uint32_t frame_offset = f.position() - moviStart - 4;
                writeFCC(f, "00dc");
                writeU32LE(f, frame_copy_len);
                writeSdChunked(f, frame_copy, frame_copy_len);
                if (frame_copy_len & 1) {
                    f.write((uint8_t)0); // 2-byte alignment padding
                }
                index_entries.push_back({frame_offset, (uint32_t)frame_copy_len});
                frame_count++;
                free(frame_copy);
            }

            uint32_t elapsed = millis() - loop_start;
            if (elapsed < frame_interval_ms) {
                vTaskDelay(pdMS_TO_TICKS(frame_interval_ms - elapsed));
            } else {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }

        // ── 4. Finalize AVI Headers & Index ──
        if (frame_count > 0) {
            uint32_t end_movi = f.position();
            patchU32(f, moviSzPos, end_movi - moviSzPos - 4);

            // Write idx1 chunk
            writeFCC(f, "idx1");
            writeU32LE(f, index_entries.size() * 16);
            for (const auto& idx : index_entries) {
                writeFCC(f, "00dc");
                writeU32LE(f, 0x10); // AVIIF_KEYFRAME
                writeU32LE(f, idx.offset);
                writeU32LE(f, idx.size);
            }

            // Patch RIFF size
            uint32_t total_file_size = f.position();
            patchU32(f, riffSzPos, total_file_size - 8);

            // Patch frame counts
            patchU32(f, avihFramesPos, frame_count);
            patchU32(f, strhFramesPos, frame_count);

            f.flush();
            f.close();

            char msg[200];
            snprintf(msg, sizeof(msg), "🎬 Video saved: %s (%uKB, %u frames)",
                     path.c_str(), total_file_size / 1024, frame_count);
            Serial.println(msg);
            telegram_send_message(msg);

            autoCleanVideos();
        } else {
            f.close();
            SD_MMC.remove(path.c_str());
        }

        xSemaphoreGive(g_sd_mutex);
    }
}

void recording_init() {
    xTaskCreatePinnedToCore(TaskRecording, "TaskRecording", 8192, nullptr, 1, nullptr, 0);
}

// ─── Uptime formatter ─────────────────────────────────────────
String formatUptime(uint32_t s) {
    uint32_t d = s / 86400; s %= 86400;
    uint32_t h = s / 3600;  s %= 3600;
    uint32_t m = s / 60;    s %= 60;
    if (d > 0) return String(d) + "d " + h + "h";
    if (h > 0) return String(h) + "h " + m + "m";
    if (m > 0) return String(m) + "m " + s + "s";
    return String(s) + "s";
}
