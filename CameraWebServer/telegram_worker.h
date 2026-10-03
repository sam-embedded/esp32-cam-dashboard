#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// ─── Job types ────────────────────────────────────────────────────────────────
typedef enum {
    TG_JOB_TEXT  = 0,   // plain text message
    TG_JOB_PHOTO = 1,   // JPEG photo from camera
    TG_JOB_VOICE = 2,   // Voice audio note via TTS
} TgJobType;

// ─── Queue message: kept small; photo is captured inside the worker task ──────
typedef struct {
    TgJobType type;
    char      text[768];        // used for TG_JOB_TEXT / TG_JOB_VOICE
    bool      capturePhoto;     // if true, worker captures frame before sending
} TgJob;

extern QueueHandle_t g_tg_queue;           // send jobs here from any task
extern volatile bool g_tg_ready;           // true once WiFi is up and task running

// ─── Public API ───────────────────────────────────────────────────────────────
void telegram_init();
void telegram_send_message(const char* text);
void telegram_send_photo(const char* caption = "");
void telegram_send_voice(const char* speech_text);
void telegram_send_voice_direct(const String& chat_id, const String& speech_text, const String& caption = "🤖 XiaoZhi Voice Output");
String telegram_test_raw_https();
String xiaozhi_ai_chat(const String& prompt);  // XiaoZhi AI processing
String xiaozhi_ai_voice_summary(const String& markdown_reply);
String xiaozhi_get_pairing_code();
String xiaozhi_regen_code();
bool   xiaozhi_is_device_linked();
void   xiaozhi_set_device_linked(bool linked);
bool   xiaozhi_verify_code(const String& input_code, const String& chat_id);
void   xiaozhi_announce_code(bool send_voice = true);
bool   xiaozhi_cloud_fetch_code();
bool   xiaozhi_cloud_poll_activate();
String xiaozhi_get_model_id();
String xiaozhi_get_model_name();
bool   xiaozhi_set_model(const String& input);
String xiaozhi_get_mcp_url();
void   xiaozhi_set_mcp_url(const String& url);
bool   xiaozhi_is_speaker_enabled();
void   xiaozhi_set_speaker_enabled(bool enabled);
void TaskTelegram(void* pvParameters);     // FreeRTOS task entry
