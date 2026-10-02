#include "ntp_sync.h"
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

static volatile bool s_ntp_synced = false;

// Notification callback fired automatically by lwIP when SNTP receives time
static void sntp_sync_notification_cb(struct timeval* tv) {
    s_ntp_synced = true;
    time_t now = tv->tv_sec;
    struct tm tinfo;
    localtime_r(&now, &tinfo);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tinfo);
    Serial.printf("[NTP] Synchronized via SNTP: %s\n", buf);
}

bool ntp_sync_time(long gmtOffsetSec, int dstOffsetSec) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[NTP] Skipped - WiFi not connected");
        return false;
    }

    // Configure SNTP notification callback
    sntp_set_time_sync_notification_cb(sntp_sync_notification_cb);

    // Set immediate sync mode so time applies right away
    esp_sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    // Multi-server anycast configuration (Google, Cloudflare, Global Pool)
    configTime(gmtOffsetSec, dstOffsetSec,
               "time.google.com",
               "time.cloudflare.com",
               "pool.ntp.org");

    // Quick initial check (up to 1.5s) in case network is immediately responsive
    uint32_t t0 = millis();
    while (millis() - t0 < 1500) {
        if (ntp_is_synchronized()) {
            return true;
        }
        delay(100);
    }

    // If not synchronized immediately, the background lwIP daemon continues polling
    if (!ntp_is_synchronized()) {
        Serial.println("[NTP] Background sync initiated...");
    }
    return ntp_is_synchronized();
}

String ntp_get_formatted_time() {
    time_t now = time(nullptr);
    if (now < 1700000000) return "--";
    struct tm tinfo;
    if (!localtime_r(&now, &tinfo)) return "--";
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tinfo);
    return String(buf);
}

String ntp_get_iso_timestamp() {
    time_t now = time(nullptr);
    if (now < 1700000000) return "";
    struct tm tinfo;
    if (!gmtime_r(&now, &tinfo)) return "";
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tinfo);
    return String(buf);
}

bool ntp_is_synchronized() {
    time_t now = time(nullptr);
    return (now > 1700000000);
}
