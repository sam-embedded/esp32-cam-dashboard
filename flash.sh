#!/bin/bash
# flash.sh  –  Auto-detect port and flash ESP32-CAM
# Usage: ./flash.sh
set -e

FQBN="esp32:esp32:esp32cam:PartitionScheme=min_spiffs"
SKETCH="/home/sam/Documents/Projects/Electronics/ESP32/cam/CameraWebServer"

# ── Find the USB serial port ──────────────────────────────────
find_port() {
    for p in /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2 /dev/ttyACM0; do
        [ -e "$p" ] && echo "$p" && return
    done
    echo ""
}

PORT=$(find_port)
if [ -z "$PORT" ]; then
    echo "❌  No USB serial port found. Is the ESP32-CAM-MB connected?"
    exit 1
fi
echo "📌 Found port: $PORT"

echo ""
echo "══════════════════════════════════════════════"
echo "  To enter FLASH mode on ESP32-CAM-MB:"
echo "  1. HOLD the IO0 button"
echo "  2. Press and release RST once"
echo "  3. Release IO0"
echo "  Then press ENTER here to start flashing..."
echo "══════════════════════════════════════════════"
read -r

echo "⚡ Flashing CameraWebServer to ESP32-CAM on $PORT ..."
arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$SKETCH"

echo ""
echo "✅  Flash complete!  Press RST on the board to boot."
