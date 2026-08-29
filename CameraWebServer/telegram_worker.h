#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// ─── Job types ────────────────────────────────────────────────────────────────
typedef enum {
    TG_JOB_TEXT  = 0,   // plain text message
    TG_JOB_PHOTO = 1,   // JPEG photo from camera
} TgJobType;

// ─── Queue message: kept small; photo is captured inside the worker task ──────
typedef struct {
    TgJobType type;
    char      text[512];        // used for TG_JOB_TEXT
    bool      capturePhoto;     // if true, worker captures frame before sending
} TgJob;

extern QueueHandle_t g_tg_queue;           // send jobs here from any task
extern volatile bool g_tg_ready;           // true once WiFi is up and task running

// ─── Public API ───────────────────────────────────────────────────────────────
void telegram_init();
void telegram_send_message(const char* text);
void telegram_send_photo(const char* caption = "");
String telegram_test_raw_https();
void TaskTelegram(void* pvParameters);     // FreeRTOS task entry
