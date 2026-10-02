// ============================================================
//  CameraWebServer.ino  –  Main sketch
//  AI-Thinker ESP32-CAM  |  esp32:esp32:esp32cam
//  FreeRTOS tasks: Stream, Telegram, WiFiWatchdog, Recording, Telemetry
// ============================================================
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <esp_system.h>
#include "esp_camera.h"
#include "camera_pins.h"
#include "board_config.h"
#include "telegram_worker.h"
#include "sd_manager.h"
#include "app_httpd.h"
#include "ntp_sync.h"
#include <lwip/dns.h>

// ─── Shared globals ───────────────────────────────────────────
Preferences        preferences;
SemaphoreHandle_t  camera_mutex = nullptr;
extern int         g_flash_pin;   // defined in app_httpd.cpp

// ─── WiFi watchdog state ──────────────────────────────────────
static volatile bool  g_wifi_connected    = false;
static volatile bool  g_ap_fallback       = false;
static volatile uint32_t g_wifi_lost_ms   = 0;

// ─── Camera init ──────────────────────────────────────────────
static bool initCamera() {
    camera_config_t config = {};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0    = Y2_GPIO_NUM;
    config.pin_d1    = Y3_GPIO_NUM;
    config.pin_d2    = Y4_GPIO_NUM;
    config.pin_d3    = Y5_GPIO_NUM;
    config.pin_d4    = Y6_GPIO_NUM;
    config.pin_d5    = Y7_GPIO_NUM;
    config.pin_d6    = Y8_GPIO_NUM;
    config.pin_d7    = Y9_GPIO_NUM;
    config.pin_xclk  = XCLK_GPIO_NUM;
    config.pin_pclk  = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href  = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn  = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;       // 20MHz standard for OV2640 + PSRAM
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode    = CAMERA_GRAB_LATEST; // Always grab the freshest frame (zero lag)
    if (psramFound()) {
        config.frame_size   = FRAMESIZE_VGA;
        config.jpeg_quality = 14;             // High speed, ~20KB per frame (smooth real-time 25fps)
        config.fb_count     = 2;              // Double buffer in PSRAM
        config.fb_location  = CAMERA_FB_IN_PSRAM;
    } else {
        config.frame_size   = FRAMESIZE_QVGA;
        config.jpeg_quality = 14;
        config.fb_count     = 1;
        config.fb_location  = CAMERA_FB_IN_DRAM;
    }
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("[CAM] Init failed: 0x%x\n", err);
        return false;
    }
    // Sensor tweaks: Load saved defaults from NVS if available
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
        int fs   = preferences.getInt("cam_framesize", FRAMESIZE_VGA);
        int qual = preferences.getInt("cam_quality", 14);
        int br   = preferences.getInt("cam_bright", 0);
        int co   = preferences.getInt("cam_contrast", 0);
        int sa   = preferences.getInt("cam_sat", 0);
        int se   = preferences.getInt("cam_effect", 0);
        int wb   = preferences.getInt("cam_wb", 0);
        int vf   = preferences.getInt("cam_vflip", 0);
        int hm   = preferences.getInt("cam_hmirror", 0);
        int awb  = preferences.getInt("cam_awb", 1);
        int aec  = preferences.getInt("cam_aec", 1);

        s->set_framesize(s, (framesize_t)fs);
        s->set_quality(s, qual);
        s->set_brightness(s, br);
        s->set_contrast(s, co);
        s->set_saturation(s, sa);
        s->set_special_effect(s, se);
        s->set_wb_mode(s, wb);
        s->set_vflip(s, vf);
        s->set_hmirror(s, hm);
        s->set_whitebal(s, awb);
        s->set_exposure_ctrl(s, aec);
    }
    Serial.println("[CAM] Initialised OK with saved settings");
    return true;
}

// ─── mDNS service init / re-announce ──────────────────────────
static void initMDNS() {
    MDNS.end();
    String hostname = preferences.getString("mdns_name", "esp32cam");
    hostname.toLowerCase();
    hostname.replace("_", "-");
    hostname.trim();
    if (hostname.isEmpty()) hostname = "esp32cam";

    if (MDNS.begin(hostname.c_str())) {
        MDNS.setInstanceName("ESP32-CAM Video Streamer");
        MDNS.addService("http", "tcp", 80);
        MDNS.addServiceTxt("http", "tcp", "version", "2.0");
        MDNS.addServiceTxt("http", "tcp", "path", "/");
        MDNS.addService("stream", "tcp", 81);
        MDNS.addServiceTxt("stream", "tcp", "path", "/stream");
        Serial.printf("[mDNS] Responding at http://%s.local\n", hostname.c_str());
    } else {
        Serial.println("[mDNS] Failed to start mDNS service");
    }
}

// ─── WiFi connection notification state ───────────────────────
static volatile bool g_send_wifi_connect_notify = false;
static bool          g_is_initial_boot_notify   = true;

// Flags set by event handler, printed safely in task context
static volatile bool    g_evt_got_ip        = false;
static volatile bool    g_evt_disconnect    = false;
static volatile uint8_t g_disconnect_reason = 0;

// ─── WiFi event handler (strictly non-blocking — no Serial/NVS/mDNS) ─
static void wifiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
            // Immediately override router DNS with 8.8.8.8 and 1.1.1.1 in lwIP
            ip_addr_t d1, d2;
            ipaddr_aton("8.8.8.8", &d1);
            ipaddr_aton("1.1.1.1", &d2);
            dns_setserver(0, &d1);
            dns_setserver(1, &d2);

            WiFi.setSleep(false); // Disable WiFi modem sleep for zero-lag streaming
            g_wifi_connected = true;
            g_ap_fallback    = false;
            g_send_wifi_connect_notify = true;
            g_evt_got_ip     = true;
            break;
        }

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            g_wifi_connected    = false;
            g_disconnect_reason = info.wifi_sta_disconnected.reason;
            g_evt_disconnect    = true;
            break;

        default: break;
    }
}

// ─── WiFi Watchdog task ───────────────────────────────────────
static void TaskWiFiWatchdog(void* pvParameters) {
    uint32_t disconnected_since = 0;
    uint32_t attempts = 0;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));  // Check every 2s for faster detection

        // Print any pending WiFi event log lines safely from this task context
        if (g_evt_got_ip) {
            g_evt_got_ip = false;
            Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        }
        if (g_evt_disconnect) {
            g_evt_disconnect = false;
            Serial.printf("[WiFi] Disconnected (reason %u) — watchdog monitoring...\n", g_disconnect_reason);
        }

        // If WiFi is connected, reset watchdog state and continue
        if (WiFi.status() == WL_CONNECTED) {
            disconnected_since = 0;
            attempts = 0;
            g_wifi_connected = true;
            continue;
        }

        g_wifi_connected = false;

        // Mark the time disconnection was first noticed
        if (disconnected_since == 0) {
            disconnected_since = millis();
            continue;
        }

        // Wait at least 15 seconds before taking action (give built-in auto-reconnect a chance)
        if (millis() - disconnected_since < 15000) {
            continue;
        }

        attempts++;
        Serial.printf("[WiFi] Watchdog: offline for %lu s, reconnect attempt %u\n",
                      (millis() - disconnected_since) / 1000, attempts);

        String ssid = preferences.getString("wifi_ssid", "FTTH");
        String pass = preferences.getString("wifi_pass", "Selva@home");
        ssid.trim();
        pass.trim();
        if (ssid.isEmpty()) ssid = "FTTH";
        if (pass.isEmpty()) pass = "Selva@home";

        WiFi.disconnect();
        vTaskDelay(pdMS_TO_TICKS(300));
        WiFi.begin(ssid.c_str(), pass.c_str());

        // Wait up to 12s for reconnect
        uint32_t wait_start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - wait_start < 12000) {
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[WiFi] Watchdog reconnected! IP: %s\n", WiFi.localIP().toString().c_str());
            disconnected_since = 0;
            attempts = 0;
            g_wifi_connected = true;
            // Trigger reconnect Telegram notification (NOT a boot notify)
            g_is_initial_boot_notify   = false;
            g_send_wifi_connect_notify = true;
            continue;
        }

        if (attempts >= 5) {
            Serial.println("[WiFi] Max retries reached – starting SoftAP fallback");
            WiFi.disconnect();
            WiFi.mode(WIFI_AP);
            WiFi.softAP("ESP32-CAM-AP", "esp32cam1234");
            g_ap_fallback = true;
            telegram_send_message("⚠️ WiFi failed after 5 retries\\nFallback AP: ESP32-CAM-AP\\nPassword: esp32cam1234");
            // Stay in AP mode for 3 minutes before retrying STA
            vTaskDelay(pdMS_TO_TICKS(180000));
            attempts = 0;
            disconnected_since = millis();
            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid.c_str(), pass.c_str());
        }
    }
}

// ─── Telemetry task ───────────────────────────────────────────
extern volatile bool g_tg_ready;  // defined in telegram_worker.cpp

static void TaskTelemetry(void* pvParameters) {
    uint32_t heartbeat = millis();
    uint32_t memCheck  = millis();

    for (;;) {
        // Heartbeat every 6 hours
        if (millis() - heartbeat > 6UL * 3600 * 1000) {
            heartbeat = millis();
            uint32_t up = millis() / 1000;
            char buf[200];
            snprintf(buf, sizeof(buf),
                "💓 Heartbeat\nUptime: %s\nIP: `%s`\nHeap: %dKB\nPSRAM: %dKB",
                formatUptime(up).c_str(),
                WiFi.localIP().toString().c_str(),
                esp_get_free_heap_size() / 1024,
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024
            );
            telegram_send_message(buf);
        }

        // Low memory alert
        if (millis() - memCheck > 60000) {
            memCheck = millis();
            uint32_t heap = esp_get_free_heap_size();
            if (heap < 20000) {
                char buf[120];
                snprintf(buf, sizeof(buf), "⚠️ Low memory alert! Free heap: %dKB", heap/1024);
                telegram_send_message(buf);
            }
        }

        // Active NTP synchronization retry every 10s until synchronized
        static uint32_t lastNtpRetry = 0;
        if (!ntp_is_synchronized() && WiFi.status() == WL_CONNECTED) {
            if (millis() - lastNtpRetry > 10000) {
                lastNtpRetry = millis();
                long gmtOffset = preferences.getLong("ntp_offset", 19800);
                int dstOffset  = preferences.getInt("ntp_dst", 0);
                ntp_sync_time(gmtOffset, dstOffset);
            }
        }

        // Automatic rich notification whenever WiFi connects or reconnects with IP
        // Wait until the Telegram task is ready before attempting to send
        if (g_send_wifi_connect_notify && WiFi.status() == WL_CONNECTED && g_tg_ready) {
            g_send_wifi_connect_notify = false;

            // Force robust public DNS servers (bypasses dead or misconfigured router DNS)
            {
                ip_addr_t d1, d2;
                ipaddr_aton("8.8.8.8", &d1);
                ipaddr_aton("1.1.1.1", &d2);
                dns_setserver(0, &d1);
                dns_setserver(1, &d2);
                Serial.println("[DNS] Overrode router DNS → 8.8.8.8 / 1.1.1.1");
            }

            // Safe mDNS registration in task context (DNS must be set first)
            initMDNS();

            // Allow SNTP up to 3s to acquire time if just connecting
            uint32_t t_sync = millis();
            while (!ntp_is_synchronized() && millis() - t_sync < 3000) {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            char sdStatus[64];
            if (sd_is_mounted()) {
                uint64_t total = 0, used = 0;
                sd_get_info(total, used);
                uint64_t free_mb = (total > used) ? (total - used) / (1024 * 1024) : 0;
                snprintf(sdStatus, sizeof(sdStatus), "Mounted (%lluMB free) ✅", free_mb);
            } else {
                snprintf(sdStatus, sizeof(sdStatus), "Not mounted ❌");
            }

            String timeStr = ntp_is_synchronized() ? (ntp_get_formatted_time() + " ✅") : String("Sync pending ⏳");
            String hostStr = preferences.getString("mdns_name", "esp32cam");
            uint32_t up = millis() / 1000;
            String ipStr = WiFi.localIP().toString();

            char notifyMsg[512];
            snprintf(notifyMsg, sizeof(notifyMsg),
                "%s\n"
                "🌐 *IP:* `%s`\n"
                "📡 *SSID:* `%s` (%d dBm)\n"
                "🔗 *Dashboard:* http://%s.local\n"
                "📹 *Live Stream:* http://%s:81/stream\n"
                "⏱️ *Time:* `%s`\n"
                "💾 *SD Card:* %s\n"
                "🧠 *Heap:* %dKB | *PSRAM:* %dKB\n"
                "⏱️ *Uptime:* %s",
                g_is_initial_boot_notify ? "🚀 *ESP32-CAM Live & Online!*" : "📶 *WiFi Reconnected & Online!*",
                ipStr.c_str(),
                WiFi.SSID().c_str(), WiFi.RSSI(),
                hostStr.c_str(),
                ipStr.c_str(),
                timeStr.c_str(),
                sdStatus,
                esp_get_free_heap_size() / 1024,
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024,
                formatUptime(up).c_str()
            );
            telegram_send_message(notifyMsg);
            g_is_initial_boot_notify = false;
        }

        // Blink status LED (GPIO33, active-low on AI-Thinker)
        static bool ledState = false;
        ledState = !ledState;
        digitalWrite(STATUS_LED_PIN, ledState ? LOW : HIGH);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ─── setup() ─────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("\n[BOOT] ESP32-CAM starting...");

    // LED pins
    pinMode(FLASH_LED_PIN,  OUTPUT); digitalWrite(FLASH_LED_PIN,  LOW);
    pinMode(STATUS_LED_PIN, OUTPUT); digitalWrite(STATUS_LED_PIN, LOW);

    // NVS
    preferences.begin("cam_config", false);

    // Camera mutex
    camera_mutex = xSemaphoreCreateMutex();

    // Camera
    if (!initCamera()) {
        Serial.println("[BOOT] Camera failed – auto-restarting in 3s");
        delay(3000);
        esp_restart();
    }

    // Pre-configure DNS override so lwIP has 8.8.8.8 and 1.1.1.1 immediately
    {
        ip_addr_t d1, d2;
        ipaddr_aton("8.8.8.8", &d1);
        ipaddr_aton("1.1.1.1", &d2);
        dns_setserver(0, &d1);
        dns_setserver(1, &d2);
    }

    // WiFi
    WiFi.persistent(false);
    WiFi.onEvent(wifiEventHandler);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    String ssid = preferences.getString("wifi_ssid", "FTTH");
    String pass = preferences.getString("wifi_pass", "Selva@home");
    ssid.trim();
    pass.trim();
    if (ssid.isEmpty()) ssid = "FTTH";
    if (pass.isEmpty()) pass = "Selva@home";

    WiFi.setHostname("esp32cam");
    Serial.printf("[WiFi] Target SSID: '%s', Pass length: %d\n", ssid.c_str(), (int)pass.length());
    Serial.printf("[WiFi] Connecting to %s", ssid.c_str());

    WiFi.begin(ssid.c_str(), pass.c_str());

    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
        delay(500); 
        Serial.print('.');
        if (g_evt_disconnect) {
            g_evt_disconnect = false;
            Serial.printf("[reason:%u]", g_disconnect_reason);
        }
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.setSleep(false);
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("[WiFi] Initial connect pending – watchdog will retry");
    }

    // mDNS will be (re-)registered by TaskTelemetry after DNS override on first GOT_IP
    // initMDNS() here is premature — DNS not yet set to 8.8.8.8, skipping.

    // Time sync (Background SNTP daemon with immediate check)
    long gmtOffset = preferences.getLong("ntp_offset", 19800); // 19800 = +5:30 (IST)
    int dstOffset  = preferences.getInt("ntp_dst", 0);
    ntp_sync_time(gmtOffset, dstOffset);
    if (ntp_is_synchronized()) {
        Serial.printf("[NTP] Time synchronized: %s\n", ntp_get_formatted_time().c_str());
    } else {
        Serial.println("[NTP] Background time sync initiated...");
    }

    // SD card (init after WiFi so Telegram is ready for SD notifications)
    sd_manager_init();

    // HTTP server (stream + API)
    startCameraServer();

    // Telegram worker
    telegram_init();

    // FreeRTOS tasks
    xTaskCreatePinnedToCore(TaskWiFiWatchdog, "TaskWiFiWD",    4096, nullptr, 3, nullptr, 0);
    xTaskCreatePinnedToCore(TaskTelemetry,    "TaskTelemetry", 8192, nullptr, 1, nullptr, 0);  // 8KB: DNS+mDNS+NTP+Telegram
    recording_init();  // TaskRecording on Core 0, priority 1

    Serial.println("[BOOT] All tasks started");
}

// ─── loop() ──────────────────────────────────────────────────
void loop() {
    delay(100);
}
