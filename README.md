# Hacktor Watch With STB

This is a custom Zephyr RTOS firmware I built to experiment with Bluetooth Low Energy (BLE) and edge computing on wearables. I started this project during the Hacktor Watch hackathon (April 2026) 

Bassicly, this project turns an ESP32-S3 smartwatch into a live public transport dashboard (currently setup for the Bucharest STB network).

## The Architecture
Smartwatches usually have tiny batteries. I realized that making the watch connect to WiFi, establish SSL, and parse massive JSON payloads from a transit API is a terrible idea for battery life and RAM. 

To fix this, I offloaded all the heavy compute to the smartphone:
1. A background daemon on the phone fetches the real-time STB API data.
2. The phone parses the JSON and formats it into a tiny, simple string like `41,Ghencea,5` (Route 41, towards Ghencea, arriving in 5 mins).
3. The phone pushes this raw string over BLE to the watch.
4. The Zephyr RTOS on the watch gets the BLE interrupt, splits the string, and directly updates the LVGL graphics.

This architecture means the watch does almost zero heavy computing, saving alot of battery and memory.

## Hardware Config
Target board: `esp32s3_devkitc/esp32s3/procpu`
- ESP32-S3 (dual core)
- GC9A01 Round LCD screen (240x240) via SPI3
- CST816T Capacitive Touch via I2C1
- Built-in ESP32 Bluetooth controller

## Code Structure
- `prj.conf` - My Zephyr Kconfig (enabled LVGL, SPI, and BLE, but disabled extra sensors to save space).
- `src/ble.c` - Sets up a custom GATT service (`12345678-1234-5678-1234-56789abcdef0`) to recive the transit string from the phone.
- `src/transit_ui.c` - Uses the LVGL library to draw a clean digital interface (tram number, destination, minutes).
- `src/main.c` - Inits the display, starts bluetooth advertising, and runs the main UI loop.
## How to build it
You need the Zephyr SDK installed. 
Just run the build script:
```bash
./build.sh --pristine
```


## Flashing

Flash the board:

```bash
./build.sh --flash --port /dev/ttyACM0
```


## Companion Mobile/Desktop Daemon

The watch acts as a pure BLE GATT server. To feed it data, the repository includes a Python bridge script (`transit_bridge.py`) meant to run on a background terminal environment (like a Linux laptop or a phone environment via Pydroid/Termux).

### Setup and Running
1. Install the asynchronous Bluetooth library dependencies:
   ```bash
   pip install bleak
