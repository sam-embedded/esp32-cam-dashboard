# 📷 ESP32-CAM Advanced Surveillance & Telemetry Firmware

A high-performance, robust, FreeRTOS-powered firmware for the AI-Thinker ESP32-CAM board with an integrated glassmorphism web dashboard, SD card video recording & file manager, real-time Telegram bot notifications, automated NTP synchronization, and dual-port HTTP server architecture.

---

## ✨ Key Features

- **Dual-Port High-FPS HTTP Server**:
  - **Port 81**: Dedicated, zero-copy MJPEG live stream server that never blocks REST API requests.
  - **Port 80**: Full REST API + Dark Glassmorphism Web Dashboard + SD Card File Manager.
- **UniversalTelegramBot & ArduinoJson Integration**:
  - Outbound event logging (boot, low memory alerts, heartbeat, OTA status).
  - Multi-user broadcast support with comma-separated Chat IDs.
  - Instant direct IPv4 SNI routing with lwIP `LOCK_TCPIP_CORE()` protection.
  - Live photo capture and camera control via `/photo`, `/flash`, `/status`, `/reboot`, `/help`.
- **SD Card File Manager & 24/7 Motion-JPEG AVI Recorder**:
  - 1-Bit SD_MMC mode avoiding Flash LED conflicts.
  - Periodic background Motion-JPEG AVI recording with RIFF headers and index chunks.
  - DMA-safe chunked SD writing (`writeSdChunked`) preventing memory errors (`0x101`).
  - Full web-based file management: folder navigation, file download, single/batch deletion, and SD formatting.
- **NTP Time Synchronization**:
  - Direct background time sync via `pool.ntp.org`, `time.nist.gov`, and `time.google.com`.
  - Live epoch timestamping for all captured videos and photos.
- **WiFi Stability & Watchdog**:
  - WiFi modem sleep disabled (`WiFi.setSleep(false)`) and RF TX power boosted to `+19.5dBm` for zero packet loss.
  - Dedicated background FreeRTOS watchdog task with auto-reconnection.
- **OTA Wireless Updates**:
  - Web-based binary upload form at `/ota` with memory-safe heap buffering.

---

## 🛠️ FreeRTOS Task Architecture

| Task Name | Core | Priority | Description |
|---|---|---|---|
| `TaskStream` | Core 1 | 5 | Dedicated high-framerate MJPEG video streaming on port 81 |
| `TaskWiFiWD` | Core 0 | 3 | Background WiFi health monitor and auto-recovery |
| `TaskTelegram` | Core 0 | 2 | Telegram queue processor and command listener |
| `TaskRecording` | Core 0 | 1 | Motion-JPEG AVI recording and auto-cleanup |
| `TaskTelemetry` | Core 0 | 1 | Status LED heartbeat, memory monitoring, and watchdog |

---

## 🚀 REST API Endpoints

- `GET /` — Dark Glassmorphism Dashboard UI
- `GET /capture` — Capture live JPEG snapshot
- `GET /control?var=...&val=...` — Adjust OV2640 camera registers
- `GET /api/telemetry` — Real-time telemetry (RSSI, Uptime, Heap, PSRAM, Time, FPS)
- `GET /api/system` — System configurations
- `POST /api/system/config` — Update WiFi, mDNS, Telegram, and Recording settings
- `GET /api/sdcard/list?path=...` — List files and subfolders on SD card
- `GET /api/sdcard/download?name=...` — Stream file to browser
- `GET /api/sdcard/delete?name=...` — Delete single or batch files
- `GET /api/sdcard/format` — Format SD card
- `POST /ota` — Web-based OTA firmware upload

---

## 📦 Build & Flash

```bash
# Compile firmware
arduino-cli compile --fqbn esp32:esp32:esp32cam:PartitionScheme=min_spiffs --export-binaries CameraWebServer

# Flash over USB (460.8k baud)
./flash.sh
```

---

## 👤 Author
- **Email**: deepsky.sam@gmail.com
- **GitHub**: [@sam-embedded](https://github.com/sam-embedded)
