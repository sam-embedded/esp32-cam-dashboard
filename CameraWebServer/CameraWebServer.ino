// ============================================================
//  CameraWebServer.ino  –  Main sketch
//  AI-Thinker ESP32-CAM  |  esp32:esp32:esp32cam
//  FreeRTOS tasks: Stream, Telegram, WiFiWatchdog, Recording, Telemetry
// ============================================================
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <ArduinoOTA.h>
#include <esp_system.h>
#include "esp_camera.h"
#include "camera_pins.h"
#include "board_config.h"
#include "telegram_worker.h"
#include "sd_manager.h"
#include "app_httpd.h"
#include "ntp_sync.h"

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
        config.jpeg_quality = 12;             // High quality, smooth fast JPEG encode
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
        int qual = preferences.getInt("cam_quality", 12);
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

// ─── WiFi event handler ───────────────────────────────────────
static void wifiEventHandler(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            g_wifi_connected = true;
            g_ap_fallback    = false;
            Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
            // Notify Telegram (queue it – might not be ready yet)
            if (g_tg_ready) {
                char buf[180];
                snprintf(buf, sizeof(buf),
                    "✅ WiFi connected!\nIP: `%s`\nURL: http://esp32cam.local\nHeap: %dKB",
                    WiFi.localIP().toString().c_str(),
                    esp_get_free_heap_size() / 1024
                );
                telegram_send_message(buf);
            }
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            if (g_wifi_connected) {
                g_wifi_connected = false;
                g_wifi_lost_ms   = millis();
                Serial.println("[WiFi] Disconnected – watchdog will reconnect");
                if (g_tg_ready) telegram_send_message("📡 WiFi disconnected! Attempting reconnect...");
            }
            break;

        default: break;
    }
}

// ─── WiFi Watchdog task ───────────────────────────────────────
static void TaskWiFiWatchdog(void* pvParameters) {
    uint32_t backoff   = 10000;   // start at 10s
    uint32_t attempts  = 0;
    uint32_t lastCheck = 0;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        if (g_wifi_connected) {
            backoff  = 10000;
            attempts = 0;
            continue;
        }
        // Disconnected: wait backoff then retry
        uint32_t lost_for = millis() - g_wifi_lost_ms;
        if (lost_for < backoff) continue;

        attempts++;
        Serial.printf("[WiFi] Reconnect attempt %u (backoff %us)\n", attempts, backoff/1000);
        String ssid = preferences.getString("wifi_ssid", "FTTH");
        String pass = preferences.getString("wifi_pass", "Selva@home");
        WiFi.disconnect(true);
        delay(500);
        WiFi.begin(ssid.c_str(), pass.c_str());

        // Exponential backoff (cap at 60s)
        backoff = min(backoff * 2, (uint32_t)60000);
        g_wifi_lost_ms = millis();  // reset timer

        if (attempts >= 5) {
            // Fall back to AP mode
            Serial.println("[WiFi] Max retries – starting SoftAP fallback");
            WiFi.disconnect(true);
            WiFi.mode(WIFI_AP);
            WiFi.softAP("ESP32-CAM-AP", "esp32cam1234");
            g_ap_fallback    = true;
            g_wifi_connected = false;
            if (g_tg_ready) telegram_send_message("⚠️ WiFi failed! Started AP: *ESP32-CAM-AP* (pass: esp32cam1234)");
            // Reset for next attempt cycle in 5 minutes
            vTaskDelay(pdMS_TO_TICKS(300000));
            attempts  = 0;
            backoff   = 10000;
            g_wifi_lost_ms = millis();
            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid.c_str(), pass.c_str());
        }
    }
}

// ─── Telemetry task ───────────────────────────────────────────
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
            if (!ntp_is_synchronized()) {
                long gmtOffset = preferences.getLong("ntp_offset", 19800);
                int dstOffset  = preferences.getInt("ntp_dst", 0);
                ntp_sync_time(gmtOffset, dstOffset);
            }
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

    // WiFi
    WiFi.onEvent(wifiEventHandler);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    String ssid = preferences.getString("wifi_ssid", "FTTH");
    String pass = preferences.getString("wifi_pass", "Selva@home");
    WiFi.setHostname("esp32cam");
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.printf("[WiFi] Connecting to %s", ssid.c_str());
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 30000) {
        delay(500); Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.setSleep(false);
        Serial.printf("[WiFi] Connected: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("[WiFi] Initial connect timed out – watchdog will retry");
    }

    // mDNS
    String hostname = preferences.getString("mdns_name", "esp32cam");
    if (MDNS.begin(hostname.c_str())) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("[mDNS] http://%s.local\n", hostname.c_str());
    }

    // Time sync (Direct UDP NTP with multi-server fallback: Google, Cloudflare, NIST)
    long gmtOffset = preferences.getLong("ntp_offset", 19800); // 19800 = +5:30 (IST)
    int dstOffset  = preferences.getInt("ntp_dst", 0);
    ntp_sync_time(gmtOffset, dstOffset);
    Serial.printf("[NTP] Time synchronized: %s\n", ntp_get_formatted_time().c_str());

    // SD card (init after WiFi so Telegram is ready for SD notifications)
    sd_manager_init();

    // HTTP server (stream + API)
    startCameraServer();

    // Telegram worker
    telegram_init();

    // FreeRTOS tasks
    xTaskCreatePinnedToCore(TaskWiFiWatchdog, "TaskWiFiWD",  4096, nullptr, 3, nullptr, 0);
    xTaskCreatePinnedToCore(TaskTelemetry,    "TaskTelemetry", 4096, nullptr, 1, nullptr, 0);
    recording_init();  // TaskRecording on Core 0, priority 1

    Serial.println("[BOOT] All tasks started");

    // Boot notification – yield to FreeRTOS scheduler while Telegram task starts
    vTaskDelay(pdMS_TO_TICKS(1500));
    char bootMsg[280];
    snprintf(bootMsg, sizeof(bootMsg),
        "🚀 *ESP32-CAM Booted!*\n"
        "🌐 IP: `%s`\n"
        "🌐 URL: http://esp32cam.local\n"
        "🧠 Heap: %dKB | PSRAM: %dKB\n"
        "📷 Camera: OV2640\n"
        "💾 SD: %s",
        WiFi.localIP().toString().c_str(),
        esp_get_free_heap_size() / 1024,
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024,
        sd_is_mounted() ? "Mounted ✅" : "Not found ❌"
    );
    telegram_send_message(bootMsg);
}

// ─── loop() ──────────────────────────────────────────────────
void loop() {
    delay(100);
}
