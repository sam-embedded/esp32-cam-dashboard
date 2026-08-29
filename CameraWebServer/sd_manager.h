#pragma once
#include <Arduino.h>

// ─── SD Manager public API ────────────────────────────────────
bool  sd_manager_init();         // mount SD_MMC in 1-bit mode; returns true on success
bool  sd_is_mounted();
void  sd_get_info(uint64_t& total_bytes, uint64_t& used_bytes);
void  sd_save_photo(uint8_t* buf, size_t len);  // saves to /photos/PHOTO_YYYYMMDD_HHMMSS.jpg

// ─── Video recording task ─────────────────────────────────────
void  recording_init();          // creates TaskRecording on Core 0
void  TaskRecording(void* pvParameters);

// ─── Uptime helpers (used by multiple files) ──────────────────
String formatUptime(uint32_t seconds);

extern SemaphoreHandle_t g_sd_mutex;
