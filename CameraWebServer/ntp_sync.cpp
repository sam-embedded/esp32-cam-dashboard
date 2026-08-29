#include "ntp_sync.h"
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

bool ntp_sync_time(long gmtOffsetSec, int dstOffsetSec) {
    if (WiFi.status() != WL_CONNECTED) return false;

    configTime(gmtOffsetSec, dstOffsetSec, "pool.ntp.org", "time.nist.gov", "time.google.com");
    
    time_t now = 0;
    struct tm timeinfo;
    uint32_t t0 = millis();
    while (millis() - t0 < 3000) {
        time(&now);
        if (now > 1700000000) {
            localtime_r(&now, &timeinfo);
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
            Serial.printf("[NTP] Time synchronized: %s\n", buf);
            return true;
        }
        delay(100);
    }
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
