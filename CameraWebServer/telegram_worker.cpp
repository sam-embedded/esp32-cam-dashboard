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
#include "lwip/tcpip.h"
#include "lwip/netdb.h"

// ─── Custom WiFiClientSecure with safe DNS resolution ──────────
class TelegramClient : public WiFiClientSecure {
public:
    TelegramClient() {
        setInsecure();
        setTimeout(10);
        setHandshakeTimeout(10);
    }

    int connect(const char* host, uint16_t port) override {
        setInsecure();

        // 1. Try direct Telegram core IPv4 endpoints (no DNS needed, no lock needed)
        static const IPAddress TG_IPV4[] = {
            IPAddress(149, 154, 167, 220),
            IPAddress(149, 154, 166, 110),
            IPAddress(91, 108, 56, 170)
        };

        for (const auto& tip : TG_IPV4) {
            // mbedTLS handles its own thread safety – no TCPIP core lock needed here
            int ret = WiFiClientSecure::connect(tip, port, host, nullptr, nullptr, nullptr);
            if (ret > 0) return ret;
        }

        // 2. DNS resolution – ONLY this call needs the TCPIP lock
        IPAddress resolvedIP;
        {
            LOCK_TCPIP_CORE();
            struct hostent* he = gethostbyname(host);
            if (he && he->h_addr_list && he->h_addr_list[0]) {
                resolvedIP = IPAddress((const uint8_t*)he->h_addr_list[0]);
            }
            UNLOCK_TCPIP_CORE();
        }

        if (resolvedIP) {
            // TLS connect without holding TCPIP lock
            return WiFiClientSecure::connect(resolvedIP, port, host, nullptr, nullptr, nullptr);
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
    UniversalTelegramBot testBot(tok, testClient);
    testBot.waitForResponse = 3000;

    uint32_t t0 = millis();
    bool ok = testBot.getMe();
    uint32_t tls_ms = millis() - t0;

    if (!ok) {
        char errBuf[256];
        snprintf(errBuf, sizeof(errBuf),
            "{\"ok\":false,\"err\":\"getMe() failed (%ums)\",\"time\":\"%s\",\"target\":\"api.telegram.org:443\"}",
            tls_ms, timeStr.c_str());
        ctx->result = String(errBuf);
    } else {
        char resBuf[512];
        snprintf(resBuf, sizeof(resBuf),
            "{\"ok\":true,\"tls_ms\":%u,\"method\":\"UniversalTelegramBot (Direct IPv4 SNI)\",\"status\":\"200 OK\",\"time\":\"%s\",\"resp\":\"@%s (%s)\"}",
            tls_ms, timeStr.c_str(), testBot.userName.c_str(), testBot.name.c_str());
        ctx->result = String(resBuf);
    }
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

    if (xSemaphoreTake(ctx.doneSem, pdMS_TO_TICKS(12000)) != pdTRUE) {
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

// ─── Command Processor for Incoming Telegram Messages ────────
static void handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = g_bot->messages[i].chat_id;
        String text    = g_bot->messages[i].text;
        text.trim();
        text.toLowerCase();

        if (!isChatAuthorized(chat_id)) {
            g_bot->sendMessage(chat_id, "⛔ *Unauthorized access.* Your Chat ID is `" + chat_id + "`.", "Markdown");
            continue;
        }

        if (text == "/photo" || text == "photo" || text == "📷 photo") {
            g_bot->sendMessage(chat_id, "📸 *Capturing live photo...*", "Markdown");

            uint8_t* jpg_buf = nullptr;
            size_t   jpg_len = 0;
            camera_fb_t* fb  = nullptr;

            if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                fb = esp_camera_fb_get();
                if (fb) {
                    jpg_buf = (uint8_t*)malloc(fb->len);
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

                free(jpg_buf);
                g_current_fb_buf = nullptr;
                g_current_fb_len = 0;
            } else {
                g_bot->sendMessage(chat_id, "❌ *Camera capture failed!* Camera busy or error.", "Markdown");
            }
        } else if (text == "/flash" || text == "flash" || text == "💡 flash") {
            int cur = digitalRead(g_flash_pin);
            int next = (cur == HIGH) ? LOW : HIGH;
            digitalWrite(g_flash_pin, next);
            g_bot->sendMessage(chat_id, (next == HIGH) ? "💡 Flash turned *ON*" : "💡 Flash turned *OFF*", "Markdown");
        } else if (text == "/flash on" || text == "flash on") {
            digitalWrite(g_flash_pin, HIGH);
            g_bot->sendMessage(chat_id, "💡 Flash turned *ON*", "Markdown");
        } else if (text == "/flash off" || text == "flash off") {
            digitalWrite(g_flash_pin, LOW);
            g_bot->sendMessage(chat_id, "💡 Flash turned *OFF*", "Markdown");
        } else if (text == "/status" || text == "status" || text == "📊 status") {
            uint32_t up = millis() / 1000;
            char buf[320];
            snprintf(buf, sizeof(buf),
                "📊 *ESP32-CAM Status*\n"
                "🌐 IP: `%s`\n"
                "📶 WiFi RSSI: `%d dBm`\n"
                "🧠 Free Heap: `%d KB`\n"
                "🧠 Free PSRAM: `%d KB`\n"
                "⏱️ Uptime: `%ud %uh %um %us`\n"
                "💾 SD Card: `%s`",
                WiFi.localIP().toString().c_str(),
                WiFi.RSSI(),
                esp_get_free_heap_size() / 1024,
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024,
                up/86400, (up%86400)/3600, (up%3600)/60, up%60,
                sd_is_mounted() ? "Mounted ✅" : "Not mounted ❌"
            );
            g_bot->sendMessage(chat_id, buf, "Markdown");
        } else if (text == "/restart" || text == "/reboot" || text == "restart" || text == "reboot") {
            g_bot->sendMessage(chat_id, "🔄 *Rebooting ESP32-CAM now...*", "Markdown");
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart();
        } else if (text.startsWith("/help") || text.startsWith("/start") || text == "help" || text == "start") {
            g_bot->sendMessage(
                chat_id,
                "🤖 *LunaCamBot Commands*\n\n"
                "📸 `/photo` — Capture & send a live photo\n"
                "💡 `/flash` — Toggle flash spotlight\n"
                "💡 `/flash on` — Turn flash ON\n"
                "💡 `/flash off` — Turn flash OFF\n"
                "📊 `/status` — Real-time telemetry & uptime\n"
                "🔄 `/reboot` — Remote restart device\n"
                "ℹ️ `/help` — Display this command menu",
                "Markdown"
            );
        }
    }
}

// ─── Main FreeRTOS Telegram Task ─────────────────────────────
void TaskTelegram(void* pvParameters) {
    tg_token = cleanToken(preferences.getString("tg_token", "8967102688:AAHEieQC2_ZHa9ci0DiPsc3O4uLclWdLJ-k"));
    String chats = preferences.getString("tg_chat_id", "318862528");
    parseChatIds(chats);

    g_bot = new UniversalTelegramBot(tg_token, g_tg_client);
    g_bot->waitForResponse = 1500;

    g_tg_ready = true;
    uint32_t pollTick = 0;

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

                if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
                    fb = esp_camera_fb_get();
                    if (fb) {
                        jpg_buf = (uint8_t*)malloc(fb->len);
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

        // ── 2. Poll incoming Telegram commands ──
        if (millis() - pollTick > 2000 || did_job) {
            pollTick = millis();
            if (WiFi.status() == WL_CONNECTED && !tg_token.isEmpty()) {
                int numNew = g_bot->getUpdates(g_bot->last_message_received + 1);
                while (numNew) {
                    handleNewMessages(numNew);
                    numNew = g_bot->getUpdates(g_bot->last_message_received + 1);
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
    g_tg_queue = xQueueCreate(20, sizeof(TgJob));
    g_tg_ready = false;
    xTaskCreatePinnedToCore(TaskTelegram, "TaskTelegram", 16384, nullptr, 2, nullptr, 0);
}
