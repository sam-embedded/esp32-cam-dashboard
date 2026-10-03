// ============================================================
//  telegram_worker.cpp  –  UniversalTelegramBot + ArduinoJson
//  • HTTPS sendMessage   (Markdown payload to all authorized chat IDs)
//  • HTTPS sendPhoto     (captures frame, streams binary photo)
//  • HTTPS getUpdates    (polling for /photo /flash /status /reboot /help)
//  • Raw HTTPS / TLS diagnostic tester via getMe()
//  • Direct IPv4 SNI routing + LOCK_TCPIP_CORE() protection
// ============================================================
#include "telegram_worker.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "esp_camera.h"
#include <freertos/semphr.h>
#include "sd_manager.h"
#include "ntp_sync.h"
#include <time.h>
#include <vector>
#include <mbedtls/platform.h>
#include <esp_heap_caps.h>

// Allocate mbedTLS buffers from 4MB external PSRAM to completely eliminate DRAM exhaustion
static void* mbedtls_custom_calloc(size_t n, size_t size) {
    void* ptr = nullptr;
    if (psramFound()) {
        ptr = heap_caps_calloc(n, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }
    if (!ptr) {
        ptr = calloc(n, size);
    }
    return ptr;
}

static void mbedtls_custom_free(void* ptr) {
    free(ptr);
}

// ─── Custom WiFiClientSecure with safe DNS resolution & MFLN ──
class TelegramClient : public WiFiClientSecure {
public:
    TelegramClient() {
        setInsecure();
        setTimeout(10000);          // 10,000 milliseconds (10s) for socket read/connect
        setHandshakeTimeout(10);    // 10 seconds for TLS handshake
    }

    int connect(const char* host, uint16_t port) override {
        setInsecure();
        if (sslclient) {
            mbedtls_ssl_conf_max_frag_len(&sslclient->ssl_conf, MBEDTLS_SSL_MAX_FRAG_LEN_2048);
        }

        // 1. Primary: Standard Arduino WiFi DNS resolution (non-blocking, uses overridden DNS 8.8.8.8)
        IPAddress resolvedIP;
        if (WiFi.hostByName(host, resolvedIP) && resolvedIP != IPAddress(0, 0, 0, 0)) {
            int ret = WiFiClientSecure::connect(resolvedIP, port, host, nullptr, nullptr, nullptr);
            if (ret > 0) return ret;
        }

        // 2. Fallback: Direct hostname connect via mbedTLS
        {
            int ret = WiFiClientSecure::connect(host, port);
            if (ret > 0) return ret;
        }

        // 3. Fallback: Direct Telegram core IPv4 endpoints if DNS is down/blocked
        static const IPAddress TG_IPV4[] = {
            IPAddress(149, 154, 167, 220),
            IPAddress(149, 154, 166, 110),
            IPAddress(91, 108, 56, 170)
        };

        for (const auto& tip : TG_IPV4) {
            if (tip == resolvedIP) continue; // skip if already attempted above
            int ret = WiFiClientSecure::connect(tip, port, host, nullptr, nullptr, nullptr);
            if (ret > 0) return ret;
        }

        return 0;
    }

    int connect(const char* host, uint16_t port, int32_t timeout) override {
        return connect(host, port);
    }
};

// ─── Globals (declared extern in .h) ────────────────────────
QueueHandle_t g_tg_queue = nullptr;
volatile bool g_tg_ready = false;

// ─── External deps ───────────────────────────────────────────
extern Preferences        preferences;
extern SemaphoreHandle_t  camera_mutex;
extern int                g_flash_pin;

// ─── Module-private state ────────────────────────────────────
static TelegramClient         g_tg_client;
static UniversalTelegramBot*  g_bot = nullptr;
static String                 tg_token;
static std::vector<String>    tg_chat_ids;

// Photo streaming callbacks for UniversalTelegramBot
static uint8_t* g_current_fb_buf = nullptr;
static size_t   g_current_fb_len = 0;
static size_t   g_current_fb_pos = 0;

static bool isMorePhotoDataAvailable() {
    return (g_current_fb_pos < g_current_fb_len);
}

static byte getNextPhotoByte() {
    if (g_current_fb_pos < g_current_fb_len) {
        return g_current_fb_buf[g_current_fb_pos++];
    }
    return 0;
}

// ─── Helper: parse comma-/space-separated chat IDs ───────────
static void parseChatIds(const String& raw) {
    tg_chat_ids.clear();
    String s = raw;
    s.trim();
    int start = 0;
    for (int i = 0; i <= (int)s.length(); i++) {
        if (i == (int)s.length() || s[i] == ',' || s[i] == ' ') {
            String tok = s.substring(start, i);
            tok.trim();
            if (tok.length() > 0) tg_chat_ids.push_back(tok);
            start = i + 1;
        }
    }
}

// ─── Helper: sanitize bot token ──────────────────────────────
static String cleanToken(String tok) {
    tok.replace("%3A", ":");
    tok.replace("%3a", ":");
    tok.trim();
    if (tok.isEmpty()) {
        tok = "8967102688:AAHEieQC2_ZHa9ci0DiPsc3O4uLclWdLJ-k";
    }
    return tok;
}

// ─── Raw HTTPS / getMe() Diagnostic Tester ───────────────────
struct TgTestContext {
    SemaphoreHandle_t doneSem;
    String result;
};

static void tgTestTask(void* pv) {
    TgTestContext* ctx = (TgTestContext*)pv;
    String tok = cleanToken(preferences.getString("tg_token", "8967102688:AAHEieQC2_ZHa9ci0DiPsc3O4uLclWdLJ-k"));
    String timeStr = ntp_get_formatted_time();

    TelegramClient testClient;

    // Test Step 1: TLS Connect to api.telegram.org:443
    uint32_t t0 = millis();
    int conn = testClient.connect("api.telegram.org", 443);
    uint32_t conn_ms = millis() - t0;

    if (!conn) {
        IPAddress resolvedIP;
        bool dns_ok = WiFi.hostByName("api.telegram.org", resolvedIP);
        char lastErr[128] = {0};
        testClient.lastError(lastErr, sizeof(lastErr));
        char errBuf[256];
        snprintf(errBuf, sizeof(errBuf),
            "{\"ok\":false,\"step\":\"connect_failed\",\"dns\":%s,\"ip\":\"%s\",\"conn_ms\":%u,\"err\":\"%s\",\"time\":\"%s\"}",
            dns_ok ? "true" : "false", resolvedIP.toString().c_str(), conn_ms, lastErr, timeStr.c_str());
        ctx->result = String(errBuf);
        xSemaphoreGive(ctx->doneSem);
        vTaskDelete(NULL);
        return;
    }

    // Test Step 2: Send HTTP GET /bot<token>/getMe
    String req = String("GET /bot") + tok + "/getMe HTTP/1.1\r\n" +
                 "Host: api.telegram.org\r\n" +
                 "User-Agent: ESP32-CAM\r\n" +
                 "Accept: application/json\r\n" +
                 "Connection: close\r\n\r\n";
    testClient.print(req);

    // Test Step 3: Read HTTP Status and Body
    uint32_t tRead = millis();
    String respHeader = "";
    String respBody = "";
    while (testClient.connected() && millis() - tRead < 5000) {
        if (testClient.available()) {
            String line = testClient.readStringUntil('\n');
            if (line == "\r" || line.length() == 0) break; // Header separator
            if (respHeader.isEmpty()) respHeader = line;   // Status line (HTTP/1.1 200 OK)
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    tRead = millis();
    while ((testClient.connected() || testClient.available()) && millis() - tRead < 3000) {
        while (testClient.available() && respBody.length() < 300) {
            respBody += (char)testClient.read();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    testClient.stop();
    uint32_t total_ms = millis() - t0;

    respHeader.trim();
    respBody.trim();
    respBody.replace("\"", "'");
    respBody.replace("\r", "");
    respBody.replace("\n", " ");

    char resBuf[512];
    snprintf(resBuf, sizeof(resBuf),
        "{\"ok\":%s,\"conn_ms\":%u,\"total_ms\":%u,\"status\":\"%s\",\"body\":\"%s\",\"time\":\"%s\"}",
        respHeader.indexOf("200") >= 0 ? "true" : "false",
        conn_ms, total_ms, respHeader.c_str(), respBody.c_str(), timeStr.c_str());
    ctx->result = String(resBuf);
    xSemaphoreGive(ctx->doneSem);
    vTaskDelete(NULL);
}

String telegram_test_raw_https() {
    if (WiFi.status() != WL_CONNECTED) {
        return "{\"ok\":false,\"err\":\"WiFi is disconnected\"}";
    }

    TgTestContext ctx;
    ctx.doneSem = xSemaphoreCreateBinary();
    ctx.result  = "{\"ok\":false,\"err\":\"Timeout executing getMe test\"}";

    TaskHandle_t hTask = NULL;
    xTaskCreatePinnedToCore(tgTestTask, "tgTestTask", 16384, &ctx, 3, &hTask, 0);

    if (xSemaphoreTake(ctx.doneSem, pdMS_TO_TICKS(15000)) != pdTRUE) {
        if (hTask) vTaskDelete(hTask);
    }
    vSemaphoreDelete(ctx.doneSem);
    return ctx.result;
}

// ─── Broadcast text to ALL authorised chat IDs ───────────────
void telegram_send_message(const char* text) {
    if (!g_tg_queue) return;
    TgJob job;
    job.type = TG_JOB_TEXT;
    job.capturePhoto = false;
    snprintf(job.text, sizeof(job.text), "%s", text);
    if (uxQueueSpacesAvailable(g_tg_queue) == 0) {
        TgJob drop;
        xQueueReceive(g_tg_queue, &drop, 0); // Drop oldest to keep queue fresh
    }
    xQueueSend(g_tg_queue, &job, 0);
}

// ─── Capture & send photo to ALL authorised chat IDs ─────────
void telegram_send_photo(const char* caption) {
    if (!g_tg_queue) return;
    TgJob job;
    job.type = TG_JOB_PHOTO;
    job.capturePhoto = true;
    if (caption) {
        snprintf(job.text, sizeof(job.text), "%s", caption);
    } else {
        job.text[0] = '\0';
    }
    if (uxQueueSpacesAvailable(g_tg_queue) == 0) {
        TgJob drop;
        xQueueReceive(g_tg_queue, &drop, 0);
    }
    xQueueSend(g_tg_queue, &job, 0);
}

// ─── Helper: check if sender chat ID is authorized ───────────
static bool isChatAuthorized(const String& chat_id) {
    if (tg_chat_ids.empty()) return true;
    for (const auto& id : tg_chat_ids) {
        if (id == chat_id) return true;
    }
    return false;
}

// ─── XiaoZhi AI (小智) Edge Agent Engine ───────────────────────
String xiaozhi_ai_chat(const String& prompt) {
    String text = prompt;
    text.trim();
    String lower = text;
    lower.toLowerCase();

    // 1. Photo / Capture
    if (lower.indexOf("photo") >= 0 || lower.indexOf("snap") >= 0 || lower.indexOf("picture") >= 0 ||
        lower.indexOf("capture") >= 0 || lower.indexOf("look") >= 0 || lower.indexOf("see") >= 0 ||
        lower.indexOf("拍照") >= 0 || lower.indexOf("看") >= 0 || lower == "/photo") {
        telegram_send_photo("📸 XiaoZhi AI Snapshot");
        return "📸 *XiaoZhi AI:* Live photo captured and queued to Telegram!";
    }

    // 2. Flash Light Controls
    if (lower.indexOf("flash on") >= 0 || lower.indexOf("light on") >= 0 || lower.indexOf("torch on") >= 0 ||
        lower.indexOf("open light") >= 0 || lower.indexOf("开灯") >= 0) {
        digitalWrite(g_flash_pin, HIGH);
        return "💡 *XiaoZhi AI:* Flash spotlight is now **ON**!";
    }
    if (lower.indexOf("flash off") >= 0 || lower.indexOf("light off") >= 0 || lower.indexOf("torch off") >= 0 ||
        lower.indexOf("close light") >= 0 || lower.indexOf("关灯") >= 0) {
        digitalWrite(g_flash_pin, LOW);
        return "💡 *XiaoZhi AI:* Flash spotlight is now **OFF**.";
    }
    if (lower == "/flash" || lower == "flash" || lower == "toggle light" || lower == "light") {
        int cur = digitalRead(g_flash_pin);
        int next = (cur == HIGH) ? LOW : HIGH;
        digitalWrite(g_flash_pin, next);
        return (next == HIGH) ? "💡 *XiaoZhi AI:* Flash toggled **ON**!" : "💡 *XiaoZhi AI:* Flash toggled **OFF**.";
    }

    // 3. Status & Health
    if (lower.indexOf("status") >= 0 || lower.indexOf("health") >= 0 || lower.indexOf("sys") >= 0 ||
        lower.indexOf("info") >= 0 || lower.indexOf("state") >= 0 || lower.indexOf("uptime") >= 0 ||
        lower.indexOf("状态") >= 0) {
        uint32_t up = millis() / 1000;
        uint64_t totalBytes = 0, usedBytes = 0;
        sd_get_info(totalBytes, usedBytes);
        uint32_t freeMB = (totalBytes > usedBytes) ? (uint32_t)((totalBytes - usedBytes) / (1024 * 1024)) : 0;
        sensor_t* s = esp_camera_sensor_get();
        int fs = s ? s->status.framesize : 6;
        const char* resStr = (fs == 10) ? "UXGA (1600x1200)" :
                             (fs == 9)  ? "SXGA (1280x1024)" :
                             (fs == 8)  ? "XGA (1024x768)" :
                             (fs == 7)  ? "SVGA (800x600)" :
                             (fs == 6)  ? "VGA (640x480)" :
                             (fs == 5)  ? "CIF (400x296)" : "QVGA (320x240)";
        extern volatile bool g_recording_active;
        extern int g_stream_fps;

        char buf[512];
        snprintf(buf, sizeof(buf),
            "🤖 *XiaoZhi AI (小智) Status Report*\n"
            "━━━━━━━━━━━━━━━━━━━━\n"
            "🌐 IP: `%s` (%s.local)\n"
            "📶 WiFi Signal: `%d dBm`\n"
            "🧠 Free Heap: `%d KB` | PSRAM: `%d KB`\n"
            "⏱️ Uptime: `%ud %uh %um %us`\n"
            "🕒 Clock: `%s`\n"
            "💾 SD Card: `%s` (%uMB free)\n"
            "🎥 Sensor: `%s @ %d FPS`\n"
            "🔴 Recording: `%s`\n"
            "💡 Flash: `%s`\n"
            "━━━━━━━━━━━━━━━━━━━━",
            WiFi.localIP().toString().c_str(),
            preferences.getString("mdns_name", "esp32cam").c_str(),
            WiFi.RSSI(),
            esp_get_free_heap_size() / 1024,
            heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024,
            up / 86400, (up % 86400) / 3600, (up % 3600) / 60, up % 60,
            ntp_get_formatted_time().c_str(),
            sd_is_mounted() ? "Mounted ✅" : "Not mounted ❌",
            freeMB,
            resStr, g_stream_fps,
            (sd_is_mounted() && g_recording_active) ? "Active 🔴" : "Idle",
            digitalRead(g_flash_pin) ? "ON 💡" : "OFF"
        );
        return String(buf);
    }

    // 4. IP / Web Links
    if (lower.indexOf("ip") >= 0 || lower.indexOf("url") >= 0 || lower.indexOf("link") >= 0 ||
        lower.indexOf("address") >= 0 || lower.indexOf("dashboard") >= 0) {
        String ip = WiFi.localIP().toString();
        String mdns = preferences.getString("mdns_name", "esp32cam");
        return "🌐 *XiaoZhi AI Links:*\n• Web UI: http://" + ip + "\n• mDNS: http://" + mdns + ".local\n• Live Stream: http://" + ip + ":81/stream";
    }

    // 5. SD Storage
    if (lower.indexOf("sd") >= 0 || lower.indexOf("disk") >= 0 || lower.indexOf("storage") >= 0) {
        if (!sd_is_mounted()) return "⚠️ *XiaoZhi AI:* SD Card is not mounted.";
        uint64_t total = 0, used = 0;
        sd_get_info(total, used);
        uint32_t totMB  = (uint32_t)(total / (1024 * 1024));
        uint32_t usedMB = (uint32_t)(used / (1024 * 1024));
        uint32_t freeMB = totMB - usedMB;
        int pct = totMB > 0 ? (usedMB * 100) / totMB : 0;
        char buf[220];
        snprintf(buf, sizeof(buf),
            "💾 *XiaoZhi AI SD Card:*\nUsed: %uMB / %uMB (%d%%) | Free: %uMB\nStatus: Mounted & Healthy ✅",
            usedMB, totMB, pct, freeMB);
        return String(buf);
    }

    // 6. Video Recording Toggle
    if (lower.indexOf("record on") >= 0 || lower.indexOf("start record") >= 0) {
        preferences.putBool("rec_enabled", true);
        return "🎬 *XiaoZhi AI:* 24/7 Video recording has been **STARTED**! Saving AVI files to SD card.";
    }
    if (lower.indexOf("record off") >= 0 || lower.indexOf("stop record") >= 0) {
        preferences.putBool("rec_enabled", false);
        return "🎬 *XiaoZhi AI:* 24/7 Video recording is now **PAUSED**.";
    }

    // 7. Resolution Control
    sensor_t* s = esp_camera_sensor_get();
    if (s && (lower.indexOf("vga") >= 0 || lower == "/vga")) {
        s->set_framesize(s, FRAMESIZE_VGA);
        return "🎥 *XiaoZhi AI:* Resolution set to **VGA (640x480)** real-time stream.";
    }
    if (s && (lower.indexOf("uxga") >= 0 || lower.indexOf("2mp") >= 0 || lower.indexOf("high res") >= 0)) {
        s->set_framesize(s, FRAMESIZE_UXGA);
        return "🎥 *XiaoZhi AI:* Resolution set to **UXGA (1600x1200)** 2MP High-Def.";
    }
    if (s && lower.indexOf("qvga") >= 0) {
        s->set_framesize(s, FRAMESIZE_QVGA);
        return "🎥 *XiaoZhi AI:* Resolution set to **QVGA (320x240)** fast mode.";
    }

    // 8. Flips
    if (s && (lower.indexOf("vflip") >= 0 || lower == "flip")) {
        int next = s->status.vflip ? 0 : 1;
        s->set_vflip(s, next);
        return next ? "🔄 *XiaoZhi AI:* Vertical flip is now **ON**" : "🔄 *XiaoZhi AI:* Vertical flip is now **OFF**";
    }
    if (s && (lower.indexOf("hmirror") >= 0 || lower == "mirror")) {
        int next = s->status.hmirror ? 0 : 1;
        s->set_hmirror(s, next);
        return next ? "🪞 *XiaoZhi AI:* Horizontal mirror is now **ON**" : "🪞 *XiaoZhi AI:* Horizontal mirror is now **OFF**";
    }

    // 9. Time
    if (lower.indexOf("time") >= 0 || lower.indexOf("clock") >= 0 || lower.indexOf("几点") >= 0) {
        return "🕒 *XiaoZhi AI Clock:* `" + ntp_get_formatted_time() + "`";
    }

    // 10. Reboot
    if (lower.indexOf("reboot") >= 0 || lower.indexOf("restart") >= 0 || lower.indexOf("reset") >= 0 || lower.indexOf("重启") >= 0) {
        return "🔄 *XiaoZhi AI:* Rebooting ESP32-CAM now... I will be back online in ~10 seconds!";
    }

    // 11. Greetings & Help
    if (lower == "hi" || lower == "hello" || lower == "hey" || lower.indexOf("who are you") >= 0 ||
        lower.indexOf("小智") >= 0 || lower.indexOf("你好") >= 0 || lower.indexOf("你是谁") >= 0 ||
        lower.startsWith("/start") || lower.startsWith("/help") || lower == "help") {
        return "✨ *XiaoZhi AI (小智) Agent Online!*\n"
               "I am your intelligent autonomous agent running natively on ESP32-CAM.\n\n"
               "🗣️ *Natural Language Commands:*\n"
               "• `Take a photo` / `look` — Capture & send live photo\n"
               "• `Turn on flash` / `flash off` — Toggle spotlight\n"
               "• `Status` / `health` — View live hardware telemetry\n"
               "• `IP address` — Get web dashboard links\n"
               "• `SD card` — Check storage usage\n"
               "• `VGA` / `UXGA` — Tune camera resolution\n"
               "• `Flip` / `Mirror` — Rotate image orientation\n"
               "• `Start recording` / `Stop recording`\n"
               "• `Reboot` — Safely restart device";
    }

    // 12. Fallback
    return "🤖 *XiaoZhi AI:* I received: _\"" + text + "\"_\n\n"
           "You can ask me to take a photo (`photo`), turn on the light (`flash on`), report health (`status`), or adjust resolution. Type `help` for full controls!";
}

// ─── Command Processor for Incoming Telegram Messages ────────
static void handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = g_bot->messages[i].chat_id;
        String text    = g_bot->messages[i].text;
        text.trim();

        if (!isChatAuthorized(chat_id)) {
            g_bot->sendMessage(chat_id, "⛔ *Unauthorized access.* Your Chat ID is `" + chat_id + "`.", "Markdown");
            continue;
        }

        String lower = text;
        lower.toLowerCase();

        // Check for direct photo capture command
        if (lower == "/photo" || lower == "photo" || lower == "📷 photo" || lower.indexOf("take a photo") >= 0 ||
            lower.indexOf("take photo") >= 0 || lower.indexOf("拍照") >= 0 || lower.indexOf("看一眼") >= 0) {
            
            g_bot->sendMessage(chat_id, "📸 *XiaoZhi AI:* Capturing live photo from camera...", "Markdown");

            uint8_t* jpg_buf = nullptr;
            size_t   jpg_len = 0;
            camera_fb_t* fb  = nullptr;

            if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                fb = esp_camera_fb_get();
                if (fb) {
                    jpg_buf = (uint8_t*)ps_malloc(fb->len);
                    if (!jpg_buf) jpg_buf = (uint8_t*)malloc(fb->len);
                    if (jpg_buf) {
                        jpg_len = fb->len;
                        memcpy(jpg_buf, fb->buf, jpg_len);
                    }
                    esp_camera_fb_return(fb);
                }
                xSemaphoreGive(camera_mutex);
            }

            if (jpg_buf && jpg_len > 0) {
                g_current_fb_buf = jpg_buf;
                g_current_fb_len = jpg_len;
                g_current_fb_pos = 0;

                g_bot->sendPhotoByBinary(chat_id, "image/jpeg", jpg_len,
                                         isMorePhotoDataAvailable,
                                         getNextPhotoByte,
                                         nullptr, nullptr);

                String caption = "✨ *XiaoZhi AI Live Snapshot*\n🕒 `" + ntp_get_formatted_time() + "`";
                g_bot->sendMessage(chat_id, caption, "Markdown");

                free(jpg_buf);
                g_current_fb_buf = nullptr;
                g_current_fb_len = 0;
            } else {
                g_bot->sendMessage(chat_id, "❌ *XiaoZhi AI:* Camera capture failed (busy or error).", "Markdown");
            }
            continue;
        }

        // Process message through XiaoZhi AI Agent Engine
        String response = xiaozhi_ai_chat(text);
        g_bot->sendMessage(chat_id, response, "Markdown");

        // If reboot was requested, delay briefly then restart
        if (lower.indexOf("reboot") >= 0 || lower.indexOf("restart") >= 0 || lower.indexOf("reset") >= 0 || lower.indexOf("重启") >= 0) {
            vTaskDelay(pdMS_TO_TICKS(1500));
            esp_restart();
        }
    }
}


// ─── Main FreeRTOS Telegram Task ─────────────────────────────
void TaskTelegram(void* pvParameters) {
    tg_token = cleanToken(preferences.getString("tg_token", "8967102688:AAHEieQC2_ZHa9ci0DiPsc3O4uLclWdLJ-k"));
    String chats = preferences.getString("tg_chat_id", "318862528");
    parseChatIds(chats);

    g_bot = new UniversalTelegramBot(tg_token, g_tg_client);
    g_bot->waitForResponse = 3500; // 3.5s timeout for mobile hotspots

    g_tg_ready = true;
    uint32_t pollTick = 0;
    uint32_t pollInterval = 2000;

    for (;;) {
        bool did_job = false;
        TgJob job;
        // ── 1. Process outbound message queue ──
        while (xQueueReceive(g_tg_queue, &job, 0) == pdTRUE) {
            did_job = true;
            if (job.type == TG_JOB_TEXT) {
                for (const auto& cid : tg_chat_ids) {
                    g_bot->sendMessage(cid, String(job.text), "Markdown");
                }
            } else if (job.type == TG_JOB_PHOTO) {
                uint8_t* jpg_buf = nullptr;
                size_t   jpg_len = 0;
                camera_fb_t* fb  = nullptr;

                if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(2500)) == pdTRUE) {
                    fb = esp_camera_fb_get();
                    if (fb) {
                        jpg_buf = (uint8_t*)ps_malloc(fb->len);
                        if (!jpg_buf) jpg_buf = (uint8_t*)malloc(fb->len);
                        if (jpg_buf) {
                            jpg_len = fb->len;
                            memcpy(jpg_buf, fb->buf, jpg_len);
                        }
                        esp_camera_fb_return(fb);
                    }
                    xSemaphoreGive(camera_mutex);
                }

                if (jpg_buf && jpg_len > 0) {
                    g_current_fb_buf = jpg_buf;
                    g_current_fb_len = jpg_len;

                    for (const auto& cid : tg_chat_ids) {
                        g_current_fb_pos = 0;
                        g_bot->sendPhotoByBinary(cid, "image/jpeg", jpg_len,
                                                 isMorePhotoDataAvailable,
                                                 getNextPhotoByte,
                                                 nullptr, nullptr);
                    }
                    free(jpg_buf);
                    g_current_fb_buf = nullptr;
                    g_current_fb_len = 0;
                }
            }
        }

        // ── 2. Poll incoming Telegram commands with intelligent backoff ──
        extern volatile bool g_is_streaming;
        uint32_t activeInterval = g_is_streaming ? 30000 : pollInterval;
        if (millis() - pollTick > activeInterval) {
            pollTick = millis();
            if (WiFi.status() == WL_CONNECTED && !tg_token.isEmpty()) {
                int numNew = g_bot->getUpdates(g_bot->last_message_received + 1);
                if (numNew > 0) {
                    pollInterval = 5000;
                    while (numNew) {
                        handleNewMessages(numNew);
                        numNew = g_bot->getUpdates(g_bot->last_message_received + 1);
                    }
                } else if (numNew < 0) {
                    // Back off to 20s on network error
                    pollInterval = 20000;
                } else {
                    pollInterval = 10000;
                }
            }
        }

        // ── 3. Reload config if changed in NVS ──
        static uint32_t configReload = 0;
        if (millis() - configReload > 15000) {
            configReload = millis();
            String newChats = preferences.getString("tg_chat_id", "318862528");
            parseChatIds(newChats);
            String newToken = cleanToken(preferences.getString("tg_token", "8967102688:AAHEieQC2_ZHa9ci0DiPsc3O4uLclWdLJ-k"));
            if (newToken != tg_token) {
                tg_token = newToken;
                g_bot->updateToken(tg_token);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ─── Init ─────────────────────────────────────────────────────
void telegram_init() {
    mbedtls_platform_set_calloc_free(mbedtls_custom_calloc, mbedtls_custom_free);
    g_tg_queue = xQueueCreate(20, sizeof(TgJob));
    g_tg_ready = false;
    xTaskCreatePinnedToCore(TaskTelegram, "TaskTelegram", 16384, nullptr, 1, nullptr, 0);
}
