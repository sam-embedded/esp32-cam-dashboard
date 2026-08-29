#pragma once
#include <Arduino.h>

bool   ntp_sync_time(long gmtOffsetSec = 19800, int dstOffsetSec = 0);
String ntp_get_formatted_time();
bool   ntp_is_synchronized();
