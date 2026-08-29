// ============================================================
//  app_httpd.cpp  –  Dual-port HTTP server
//  • Port 81: Dedicated MJPEG stream (zero-copy, high FPS, never blocked)
//  • Port 80: Full REST API + Modern Responsive Dashboard + SD File Manager
// ============================================================
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"
#include "html_ui.h"
#include "telegram_worker.h"
#include "sd_manager.h"
#include "ntp_sync.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <SD_MMC.h>
#include <Update.h>
#include <freertos/semphr.h>

// ─── External deps ────────────────────────────────────────────
extern Preferences       preferences;
extern SemaphoreHandle_t camera_mutex;
int                      g_flash_pin = 4;   // GPIO4 on AI-Thinker
static int               g_stream_fps = 25; // Default FPS
static httpd_handle_t camera_httpd = nullptr;  // port 80 – UI + API
static httpd_handle_t stream_httpd = nullptr;  // port 81 – MJPEG only

// ─── Global URL Decoder Helper ───────────────────────────────
static String urlDecode(const char* src) {
    if (!src) return "";
    String decoded = "";
    decoded.reserve(strlen(src) + 4);
    char a, b;
    size_t len = strlen(src);
    for (size_t i = 0; i < len; i++) {
        if (src[i] == '%' && i + 2 < len) {
            a = src[i + 1];
            b = src[i + 2];
            if (isxdigit(a) && isxdigit(b)) {
                if (a >= 'a') a -= 'a' - 'A';
                if (a >= 'A') a -= ('A' - 10);
                else a -= '0';
                if (b >= 'a') b -= 'a' - 'A';
                if (b >= 'A') b -= ('A' - 10);
                else b -= '0';
                decoded += (char)(16 * a + b);
                i += 2;
                continue;
            }
        }
        if (src[i] == '+') {
            decoded += ' ';
        } else {
            decoded += src[i];
        }
    }
    return decoded;
}

// ─── MJPEG stream ─────────────────────────────────────────────
#define PART_BOUNDARY "ESP32CAMStream"
static const char* STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART     =
    "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %lu\r\n\r\n";

static esp_err_t stream_handler(httpd_req_t* req) {
    esp_err_t res = ESP_OK;
    char part_buf[128];

    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");

    while (true) {
        camera_fb_t* fb = nullptr;

        if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
            fb = esp_camera_fb_get();
            if (!fb) {
                xSemaphoreGive(camera_mutex);
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
            if (res == ESP_OK) {
                size_t hlen = snprintf(part_buf, sizeof(part_buf), STREAM_PART, fb->len, millis());
                res = httpd_resp_send_chunk(req, part_buf, hlen);
            }
            if (res == ESP_OK) {
                res = httpd_resp_send_chunk(req, (const char*)fb->buf, fb->len);
            }

            esp_camera_fb_return(fb);
            xSemaphoreGive(camera_mutex);
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        if (res != ESP_OK) break;   // Client closed connection or network dropped

        // Configurable FPS pacing (e.g. 25fps = 40ms)
        int delay_ms = 1000 / (g_stream_fps > 0 ? g_stream_fps : 25);
        if (delay_ms < 10) delay_ms = 10;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }

    httpd_resp_send_chunk(req, nullptr, 0);
    return ESP_OK;
}

// ─── Dashboard HTML ───────────────────────────────────────────
static esp_err_t index_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, index_html, strlen(index_html));
}

// ─── JPEG capture + SD save ───────────────────────────────────
static esp_err_t capture_handler(httpd_req_t* req) {
    camera_fb_t* fb = nullptr;
    if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
        fb = esp_camera_fb_get();
    }
    if (!fb) {
        xSemaphoreGive(camera_mutex);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    // Save to SD
    sd_save_photo(fb->buf, fb->len);
    // Also queue Telegram photo
    telegram_send_photo("📸 Manual capture");

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment; filename=\"capture.jpg\"");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    esp_err_t res = httpd_resp_send(req, (const char*)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    xSemaphoreGive(camera_mutex);
    return res;
}

// ─── Camera control (/control?var=X&val=Y) ───────────────────
static esp_err_t control_handler(httpd_req_t* req) {
    char buf[128] = {};
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > sizeof(buf)) buf_len = sizeof(buf);
    httpd_req_get_url_query_str(req, buf, buf_len);

    char var[32] = {}, val[16] = {};
    httpd_query_key_value(buf, "var", var, sizeof(var));
    httpd_query_key_value(buf, "val", val, sizeof(val));
    int value = atoi(val);

    if (!strcmp(var, "fps")) {
        if (value >= 1 && value <= 30) {
            g_stream_fps = value;
        }
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        return httpd_resp_send(req, nullptr, 0);
    }

    sensor_t* s = esp_camera_sensor_get();
    if (!s) { httpd_resp_send_500(req); return ESP_FAIL; }

    if      (!strcmp(var, "framesize"))     s->set_framesize(s, (framesize_t)value);
    else if (!strcmp(var, "quality"))       s->set_quality(s, value);
    else if (!strcmp(var, "brightness"))    s->set_brightness(s, value);
    else if (!strcmp(var, "contrast"))      s->set_contrast(s, value);
    else if (!strcmp(var, "saturation"))    s->set_saturation(s, value);
    else if (!strcmp(var, "special_effect"))s->set_special_effect(s, value);
    else if (!strcmp(var, "wb_mode"))       s->set_wb_mode(s, value);
    else if (!strcmp(var, "awb"))           s->set_whitebal(s, value);
    else if (!strcmp(var, "awb_gain"))      s->set_awb_gain(s, value);
    else if (!strcmp(var, "aec"))           s->set_exposure_ctrl(s, value);
    else if (!strcmp(var, "aec2"))          s->set_aec2(s, value);
    else if (!strcmp(var, "ae_level"))      s->set_ae_level(s, value);
    else if (!strcmp(var, "aec_value"))     s->set_aec_value(s, value);
    else if (!strcmp(var, "agc"))           s->set_gain_ctrl(s, value);
    else if (!strcmp(var, "agc_gain"))      s->set_agc_gain(s, value);
    else if (!strcmp(var, "gainceiling"))   s->set_gainceiling(s, (gainceiling_t)value);
    else if (!strcmp(var, "bpc"))           s->set_bpc(s, value);
    else if (!strcmp(var, "wpc"))           s->set_wpc(s, value);
    else if (!strcmp(var, "raw_gma"))       s->set_raw_gma(s, value);
    else if (!strcmp(var, "lenc"))          s->set_lenc(s, value);
    else if (!strcmp(var, "vflip"))         s->set_vflip(s, value);
    else if (!strcmp(var, "hmirror"))       s->set_hmirror(s, value);
    else if (!strcmp(var, "dcw"))           s->set_dcw(s, value);
    else if (!strcmp(var, "colorbar"))      s->set_colorbar(s, value);
    else if (!strcmp(var, "flash")) {
        digitalWrite(g_flash_pin, value ? HIGH : LOW);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, nullptr, 0);
}

// ─── Flash toggle ─────────────────────────────────────────────
static esp_err_t flash_handler(httpd_req_t* req) {
    char query[32] = {};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    char state[8] = {};
    httpd_query_key_value(query, "state", state, sizeof(state));
    int s = atoi(state);
    digitalWrite(g_flash_pin, s ? HIGH : LOW);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, s ? "1" : "0", 1);
}

// ─── Telemetry JSON ───────────────────────────────────────────
static esp_err_t telemetry_handler(httpd_req_t* req) {
    uint32_t up = millis() / 1000;
    sensor_t* s = esp_camera_sensor_get();
    int fs = s ? s->status.framesize : 6;

    char json[512];
    snprintf(json, sizeof(json),
        "{"
        "\"rssi\":%d,"
        "\"uptime\":%lu,"
        "\"heap\":%lu,"
        "\"psram\":%lu,"
        "\"ip\":\"%s\","
        "\"mdns\":\"%s\","
        "\"sd_mounted\":%s,"
        "\"framesize\":%d,"
        "\"fps\":%d,"
        "\"flash\":%d,"
        "\"time\":\"%s\""
        "}",
        WiFi.RSSI(),
        (unsigned long)up,
        (unsigned long)esp_get_free_heap_size(),
        (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        WiFi.localIP().toString().c_str(),
        preferences.getString("mdns_name", "esp32cam").c_str(),
        sd_is_mounted() ? "true" : "false",
        fs,
        g_stream_fps,
        digitalRead(g_flash_pin),
        ntp_get_formatted_time().c_str()
    );
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json, strlen(json));
}

// ─── System config GET ────────────────────────────────────────
static esp_err_t system_get_handler(httpd_req_t* req) {
    String timeStr = ntp_get_formatted_time();

    char json[768];
    snprintf(json, sizeof(json),
        "{"
        "\"mdns\":\"%s\","
        "\"ssid\":\"%s\","
        "\"tg_token\":\"%s\","
        "\"tg_chat_id\":\"%s\","
        "\"rec_enabled\":%s,"
        "\"rec_interval\":%u,"
        "\"fps\":%d,"
        "\"ntp_server1\":\"%s\","
        "\"ntp_server2\":\"%s\","
        "\"ntp_offset\":%ld,"
        "\"ntp_dst\":%d,"
        "\"system_time\":\"%s\""
        "}",
        preferences.getString("mdns_name",    "esp32cam").c_str(),
        preferences.getString("wifi_ssid",    "FTTH").c_str(),
        preferences.getString("tg_token",     "8967102688:AAHEieQC2_ZHa9ci0DiPsc3O4uLclWdLJ-k").c_str(),
        preferences.getString("tg_chat_id",   "318862528").c_str(),
        preferences.getBool("rec_enabled",    true) ? "true" : "false",
        preferences.getUInt("rec_interval",   15),
        preferences.getInt("cam_fps",         25),
        preferences.getString("ntp_server1",  "pool.ntp.org").c_str(),
        preferences.getString("ntp_server2",  "time.nist.gov").c_str(),
        preferences.getLong("ntp_offset",     19800),
        preferences.getInt("ntp_dst",         0),
        timeStr.c_str()
    );
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json, strlen(json));
}

// ─── System config POST ───────────────────────────────────────
static esp_err_t system_config_handler(httpd_req_t* req) {
    char body[800] = {};
    int  received  = httpd_req_recv(req, body, sizeof(body) - 1);
    if (received <= 0) { httpd_resp_send_500(req); return ESP_FAIL; }
    body[received] = 0;

    auto getParam = [&](const char* key, char* out, size_t outLen) {
        char search[64];
        snprintf(search, sizeof(search), "%s=", key);
        char* p = strstr(body, search);
        if (!p) { out[0] = 0; return; }
        p += strlen(search);
        char* end = strchr(p, '&');
        size_t len = end ? (size_t)(end - p) : strlen(p);
        if (len >= outLen) len = outLen - 1;
        strncpy(out, p, len);
        out[len] = 0;
    };

    char mdns[64] = {}, ssid[64] = {}, pass[64] = {};
    char token[120] = {}, chats[120] = {}, rec_en[4] = {}, rec_iv[8] = {};
    char ntp1[64] = {}, ntp2[64] = {}, ntp_off[16] = {}, ntp_dst[8] = {};
    getParam("mdns_name",    mdns,   sizeof(mdns));
    getParam("wifi_ssid",    ssid,   sizeof(ssid));
    getParam("wifi_pass",    pass,   sizeof(pass));
    getParam("tg_token",     token,  sizeof(token));
    getParam("tg_chat_id",   chats,  sizeof(chats));
    getParam("rec_enabled",  rec_en, sizeof(rec_en));
    getParam("rec_interval", rec_iv, sizeof(rec_iv));
    getParam("ntp_server1",  ntp1,   sizeof(ntp1));
    getParam("ntp_server2",  ntp2,   sizeof(ntp2));
    getParam("ntp_offset",   ntp_off,sizeof(ntp_off));
    getParam("ntp_dst",      ntp_dst,sizeof(ntp_dst));

    if (mdns[0])    preferences.putString("mdns_name",   urlDecode(mdns));
    if (ssid[0])    preferences.putString("wifi_ssid",   urlDecode(ssid));
    if (pass[0])    preferences.putString("wifi_pass",   urlDecode(pass));
    if (token[0])   preferences.putString("tg_token",    urlDecode(token));
    if (chats[0])   preferences.putString("tg_chat_id",  urlDecode(chats));
    if (rec_en[0])  preferences.putBool("rec_enabled",   atoi(rec_en) != 0);
    if (rec_iv[0])  preferences.putUInt("rec_interval",  atoi(rec_iv));
    if (ntp1[0])    preferences.putString("ntp_server1", urlDecode(ntp1));
    if (ntp2[0])    preferences.putString("ntp_server2", urlDecode(ntp2));
    if (ntp_off[0]) preferences.putLong("ntp_offset",    atol(ntp_off));
    if (ntp_dst[0]) preferences.putInt("ntp_dst",        atoi(ntp_dst));

    // If NTP settings updated, reconfigure clock
    if (ntp1[0] || ntp_off[0] || ntp_dst[0]) {
        long off = preferences.getLong("ntp_offset", 19800);
        int dst = preferences.getInt("ntp_dst", 0);
        ntp_sync_time(off, dst);
    }

    // If WiFi changed, schedule reconnect
    if (ssid[0]) {
        delay(200);
        WiFi.disconnect(true);
        WiFi.begin(ssid, pass);
    }
    // If mDNS changed, restart MDNS
    if (mdns[0]) {
        MDNS.end();
        MDNS.begin(mdns);
        MDNS.addService("http", "tcp", 80);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "{\"ok\":true}", 10);
}

// ─── Restart handler ─────────────────────────────────────────
static esp_err_t restart_handler(httpd_req_t* req) {
    char query[64] = {};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    char type[16] = {};
    httpd_query_key_value(query, "type", type, sizeof(type));

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, "{\"ok\":true}", 10);

    if (!strcmp(type, "erase_nvs")) {
        telegram_send_message("🔄 NVS erased – rebooting!");
        delay(500);
        preferences.clear();
    } else {
        telegram_send_message("🔄 Device restarting (user requested)");
    }
    delay(800);
    esp_restart();
    return ESP_OK;
}

// ─── SD card info ─────────────────────────────────────────────
static esp_err_t sd_info_handler(httpd_req_t* req) {
    uint64_t total = 0, used = 0;
    sd_get_info(total, used);
    char json[128];
    snprintf(json, sizeof(json),
        "{\"total\":%llu,\"used\":%llu,\"free\":%llu,\"mounted\":%s}",
        total / 1024, used / 1024, (total - used) / 1024,
        sd_is_mounted() ? "true" : "false"
    );
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json, strlen(json));
}

// ─── SD card file list (Full recursive/path browsing) ─────────
static esp_err_t sd_list_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (!sd_is_mounted()) {
        return httpd_resp_send(req, "{\"files\":[],\"path\":\"/\"}", 25);
    }

    char query[256] = {};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    char raw_path[128] = "/";
    if (strlen(query) > 0) {
        httpd_query_key_value(query, "path", raw_path, sizeof(raw_path));
    }

    String path = urlDecode(raw_path);
    path.trim();
    if (path.isEmpty() || !path.startsWith("/")) {
        path = "/" + path;
    }
    if (path.length() > 1 && path.endsWith("/")) {
        path = path.substring(0, path.length() - 1);
    }

    String json = "{\"path\":\"" + path + "\",\"files\":[";
    bool first = true;

    File root = SD_MMC.open(path.c_str());
    if (root && root.isDirectory()) {
        File file = root.openNextFile();
        while (file) {
            if (!first) json += ",";
            first = false;
            json += "{\"name\":\"";
            String fname = String(file.name());
            int lastSlash = fname.lastIndexOf('/');
            String baseName = (lastSlash >= 0) ? fname.substring(lastSlash + 1) : fname;
            json += baseName;
            json += "\",\"path\":\"";
            if (fname.startsWith("/")) {
                json += fname;
            } else {
                if (path == "/") json += "/" + fname;
                else json += path + "/" + fname;
            }
            json += "\",\"is_dir\":";
            json += file.isDirectory() ? "true" : "false";
            json += ",\"size\":";
            json += String(file.size());
            json += "}";
            file = root.openNextFile();
        }
        root.close();
    }

    json += "]}";
    return httpd_resp_send(req, json.c_str(), json.length());
}

// ─── SD card delete (Single, batch, and folder support) ───────
static esp_err_t sd_delete_handler(httpd_req_t* req) {
    char query[512] = {};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    char raw_name[384] = {};
    if (httpd_query_key_value(query, "name", raw_name, sizeof(raw_name)) != ESP_OK) {
        httpd_query_key_value(query, "path", raw_name, sizeof(raw_name));
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    if (!sd_is_mounted() || !raw_name[0]) {
        return httpd_resp_send(req, "{\"ok\":false,\"err\":\"No file specified\"}", 38);
    }

    String s = urlDecode(raw_name);
    if (xSemaphoreTake(g_sd_mutex, pdMS_TO_TICKS(3000)) != pdTRUE) {
        return httpd_resp_send(req, "{\"ok\":false,\"err\":\"SD card busy\"}", 32);
    }

    int start = 0;
    for (int i = 0; i <= (int)s.length(); i++) {
        if (i == (int)s.length() || s[i] == ',') {
            String item = s.substring(start, i);
            item.trim();
            if (item.length() > 0) {
                if (!item.startsWith("/")) item = "/" + item;
                Serial.printf("[SD] Deleting item: %s\n", item.c_str());
                if (SD_MMC.exists(item.c_str())) {
                    File f = SD_MMC.open(item.c_str());
                    if (f && f.isDirectory()) {
                        f.close();
                        SD_MMC.rmdir(item.c_str());
                    } else {
                        if (f) f.close();
                        SD_MMC.remove(item.c_str());
                    }
                } else {
                    SD_MMC.remove(item.c_str());
                }
            }
            start = i + 1;
        }
    }
    xSemaphoreGive(g_sd_mutex);
    return httpd_resp_send(req, "{\"ok\":true}", 11);
}

// ─── SD card download / inline preview ────────────────────────
static esp_err_t sd_download_handler(httpd_req_t* req) {
    char query[256] = {};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    char raw_name[128] = {};
    httpd_query_key_value(query, "name", raw_name, sizeof(raw_name));

    if (!sd_is_mounted() || !raw_name[0]) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    String path = urlDecode(raw_name);
    if (!path.startsWith("/")) path = "/" + path;

    File f = SD_MMC.open(path.c_str(), FILE_READ);
    if (!f || f.isDirectory()) {
        if (f) f.close();
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    // Determine Content-Type
    String lname = path;
    lname.toLowerCase();
    const char* ct = lname.endsWith(".avi") ? "video/avi" :
                     lname.endsWith(".jpg") || lname.endsWith(".jpeg") ? "image/jpeg" :
                     lname.endsWith(".png") ? "image/png" :
                     lname.endsWith(".txt") || lname.endsWith(".log") ? "text/plain" :
                     "application/octet-stream";
    httpd_resp_set_type(req, ct);

    char inline_mode[8] = {};
    httpd_query_key_value(query, "inline", inline_mode, sizeof(inline_mode));
    if (strcmp(inline_mode, "1") != 0 && !lname.endsWith(".jpg") && !lname.endsWith(".jpeg") && !lname.endsWith(".png")) {
        int lastSlash = path.lastIndexOf('/');
        String baseName = (lastSlash >= 0) ? path.substring(lastSlash + 1) : path;
        String disp = "attachment; filename=\"" + baseName + "\"";
        httpd_resp_set_hdr(req, "Content-Disposition", disp.c_str());
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    uint8_t buf[4096];
    size_t  bytes;
    while ((bytes = f.read(buf, sizeof(buf))) > 0) {
        if (httpd_resp_send_chunk(req, (const char*)buf, bytes) != ESP_OK) break;
    }
    f.close();
    httpd_resp_send_chunk(req, nullptr, 0);
    return ESP_OK;
}

// ─── SD card format ───────────────────────────────────────────
static esp_err_t sd_format_handler(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    if (!sd_is_mounted()) {
        return httpd_resp_send(req, "{\"ok\":false,\"msg\":\"not mounted\"}", 32);
    }
    telegram_send_message("⚠️ SD card format requested by user!");
    SD_MMC.end();
    // Re-mount which will format via mkfs if needed
    bool ok = SD_MMC.begin("/sdcard", true);
    if (ok) {
        SD_MMC.mkdir("/photos");
        SD_MMC.mkdir("/videos");
    }
    return httpd_resp_send(req, ok ? "{\"ok\":true}" : "{\"ok\":false}", ok ? 11 : 12);
}

// ─── Web OTA upload handler ───────────────────────────────────
static esp_err_t ota_post_handler(httpd_req_t* req) {
    size_t remaining = req->content_len;
    if (remaining == 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    Serial.printf("[OTA] Starting Web OTA update, size: %u bytes\n", remaining);

    uint8_t* buf = (uint8_t*)malloc(2048);
    if (!buf) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        free(buf);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    size_t written = 0;
    while (remaining > 0) {
        size_t toRead = min((size_t)2048, remaining);
        int received = httpd_req_recv(req, (char*)buf, toRead);
        if (received <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) continue;
            Update.abort();
            free(buf);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (Update.write(buf, received) != (size_t)received) {
            Update.abort();
            free(buf);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        written   += received;
        remaining -= received;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    free(buf);

    if (Update.end(true)) {
        Serial.printf("[OTA] Web OTA success! Written: %u bytes\n", written);
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, "{\"ok\":true,\"msg\":\"Rebooting...\"}", 31);
        delay(800);
        esp_restart();
        return ESP_OK;
    } else {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
}

// ─── Save camera settings as default ──────────────────────────
static esp_err_t camera_save_handler(httpd_req_t* req) {
    sensor_t* s = esp_camera_sensor_get();
    if (!s) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    preferences.putInt("cam_framesize", (int)s->status.framesize);
    preferences.putInt("cam_quality",   (int)s->status.quality);
    preferences.putInt("cam_bright",    (int)s->status.brightness);
    preferences.putInt("cam_contrast",  (int)s->status.contrast);
    preferences.putInt("cam_sat",       (int)s->status.saturation);
    preferences.putInt("cam_effect",    (int)s->status.special_effect);
    preferences.putInt("cam_wb",        (int)s->status.wb_mode);
    preferences.putInt("cam_vflip",     (int)s->status.vflip);
    preferences.putInt("cam_hmirror",   (int)s->status.hmirror);
    preferences.putInt("cam_awb",       (int)s->status.awb);
    preferences.putInt("cam_aec",       (int)s->status.aec);
    preferences.putInt("cam_fps",       g_stream_fps);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "{\"ok\":true}", 11);
}

// ─── Telegram test handlers ───────────────────────────────────
static esp_err_t telegram_test_msg_handler(httpd_req_t* req) {
    telegram_send_message("🧪 *Test Message* from ESP32-CAM Dashboard!\nYour Telegram bot is fully working and connected.");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "{\"ok\":true}", 11);
}

static esp_err_t telegram_test_photo_handler(httpd_req_t* req) {
    telegram_send_photo("📸 Test Snapshot from ESP32-CAM Dashboard");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "{\"ok\":true}", 11);
}

static esp_err_t telegram_test_https_handler(httpd_req_t* req) {
    String res = telegram_test_raw_https();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, res.c_str(), res.length());
}

// ─── startCameraServer() ─────────────────────────────────────
void startCameraServer() {
    g_stream_fps = preferences.getInt("cam_fps", 25);

    // ── Port 81: MJPEG stream server (dedicated, never blocks port 80) ──
    httpd_config_t scfg  = HTTPD_DEFAULT_CONFIG();
    scfg.server_port     = 81;
    scfg.ctrl_port       = 32768;   // unique ctrl socket port
    scfg.max_uri_handlers = 2;
    scfg.max_open_sockets = 3;
    scfg.stack_size      = 8192;
    scfg.task_priority   = 5;
    scfg.core_id         = 1;
    scfg.recv_wait_timeout = 5;
    scfg.send_wait_timeout = 5;
    scfg.lru_purge_enable = true;
    if (httpd_start(&stream_httpd, &scfg) == ESP_OK) {
        httpd_uri_t su = { "/stream", HTTP_GET, stream_handler, nullptr };
        httpd_register_uri_handler(stream_httpd, &su);
        Serial.println("[HTTP] Stream server on port 81");
    } else {
        Serial.println("[HTTP] Stream server FAILED");
    }

    // ── Port 80: API + UI server ────────────────────────────────
    httpd_config_t config    = HTTPD_DEFAULT_CONFIG();
    config.server_port       = 80;
    config.ctrl_port         = 32769;   // different ctrl socket port
    config.max_uri_handlers  = 24;
    config.max_open_sockets  = 7;
    config.stack_size        = 8192;
    config.task_priority     = 4;
    config.core_id           = 1;
    config.recv_wait_timeout = 30;
    config.send_wait_timeout = 30;
    config.lru_purge_enable  = true;

    auto reg = [&](const char* uri, httpd_method_t method, esp_err_t (*handler)(httpd_req_t*)) {
        httpd_uri_t u = { uri, method, handler, nullptr };
        httpd_register_uri_handler(camera_httpd, &u);
    };

    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        reg("/",                    HTTP_GET,  index_handler);
        reg("/capture",             HTTP_GET,  capture_handler);
        reg("/control",             HTTP_GET,  control_handler);
        reg("/api/telemetry",       HTTP_GET,  telemetry_handler);
        reg("/api/system",          HTTP_GET,  system_get_handler);
        reg("/api/system/config",   HTTP_POST, system_config_handler);
        reg("/api/system/flash",    HTTP_GET,  flash_handler);
        reg("/api/system/restart",  HTTP_GET,  restart_handler);
        reg("/api/camera/save",     HTTP_POST, camera_save_handler);
        reg("/api/telegram/test_msg",   HTTP_POST, telegram_test_msg_handler);
        reg("/api/telegram/test_photo", HTTP_POST, telegram_test_photo_handler);
        reg("/api/telegram/test_https", HTTP_GET,  telegram_test_https_handler);
        reg("/api/sdcard/info",     HTTP_GET,  sd_info_handler);
        reg("/api/sdcard/list",     HTTP_GET,  sd_list_handler);
        reg("/api/sdcard/delete",   HTTP_GET,  sd_delete_handler);
        reg("/api/sdcard/download", HTTP_GET,  sd_download_handler);
        reg("/api/sdcard/format",   HTTP_GET,  sd_format_handler);
        reg("/ota",                 HTTP_POST, ota_post_handler);
        Serial.println("[HTTP] API server on port 80");
    } else {
        Serial.println("[HTTP] API server FAILED");
    }
}
