#include "ntp_sync.h"
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

bool ntp_sync_time(long gmtOffsetSec, int dstOffsetSec) {
    if (WiFi.status() != WL_CONNECTED) return false;

    configTime(gmtOffsetSec, dstOffsetSec, "pool.ntp.org", "time.nist.gov", "time.google.com");

    // Try up to 2 attempts with 5s each
    for (int attempt = 0; attempt < 2; attempt++) {
        time_t now = 0;
        struct tm timeinfo;
        uint32_t t0 = millis();
        while (millis() - t0 < 5000) {
            time(&now);
            if (now > 1700000000) {
                localtime_r(&now, &timeinfo);
                char buf[64];
                strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
                Serial.printf("[NTP] Synchronized: %s\n", buf);
                return true;
            }
            delay(200);
        }
        if (attempt == 0) {
            // Retry with alternative servers
            configTime(gmtOffsetSec, dstOffsetSec, "time.cloudflare.com", "pool.ntp.org", "time.google.com");
        }
    }
    Serial.println("[NTP] Sync timed out");
    return false;
}

String ntp_get_formatted_time() {
    time_t now = time(nullptr);
    if (now < 100000) return "--";
    struct tm tinfo;
    if (!localtime_r(&now, &tinfo)) return "--";
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tinfo);
    return String(buf);
}

bool ntp_is_synchronized() {
    return (time(nullptr) > 1700000000);
}
