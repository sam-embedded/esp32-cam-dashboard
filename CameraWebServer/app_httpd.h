#pragma once
#include <Arduino.h>

// HTTP server: streaming + REST API
void startCameraServer();

// Declared extern so app_httpd.cpp can access them
extern int g_flash_pin;
