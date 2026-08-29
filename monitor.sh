#!/bin/bash
# monitor.sh  –  Auto-detect port and open serial monitor

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

echo "==> Serial monitor on $PORT at 115200 baud"
echo "    Press Ctrl+] to exit"
echo ""
python3 -m serial.tools.miniterm --rts 0 --dtr 0 "$PORT" 115200
