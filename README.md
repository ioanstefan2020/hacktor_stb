# Hacktor STB Transit Watch 🚃

This is a custom firmware I made for the Hacktor Watch built on Zephyr RTOS. Im really passioante about embedded systems and wanted to do something different than just a normal fitness tracker or analog clock. 

Bassicly, this turns the watch into a live public transport (STB) dashboard for Bucharest. 

## The Idea (Edge Computing)
I realized that making a small ESP32-S3 watch connect to WiFi and parse massive JSON payloads from the STB API is a bad idea for battery life and RAM. So I decided to offload the compute to the smartphone:
1. My phone fetches the real-time STB API data.
2. The phone formats it into a tiny, simple string like `41,Ghencea,5` (Route 41, towards Ghencea, arriving in 5 mins).
3. The phone sends this string over Bluetooth Low Energy (BLE) to the watch.
4. The Zephyr RTOS on the watch gets the BLE interrupt, splits the string, and just updates the text on the screen.

This way the watch does almost no heavy computing and saves alot of battery!

## Hardware
Target board: `esp32s3_devkitc/esp32s3/procpu`
- ESP32-S3 (dual core)
- GC9A01 Round LCD screen (240x240) via SPI
- Built-in ESP32 Bluetooth controller

## Code Structure
I gutted out all the unnecessary sensor code from the base hackathon project to keep it clean and focused just on the transit display:
- `prj.conf` - My Zephyr config (enabled LVGL, SPI, BLE, but disabled all the IMU/Haptic/Battery stuff to save space)
- `src/ble.c` - Sets up a custom GATT service (`12345678-1234-5678-1234-56789abcdef0`) to recive the transit string from the phone.
- `src/transit_ui.c` - Uses the LVGL library to draw the clean digital interface (tram number, destination, minutes).
- `src/main.c` - Inits the display, starts bluetooth advertising, and runs the UI loop.

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


