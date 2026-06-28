# Hacktor Basic

Small Zephyr application for the `Hacktor Watch` that brings up a round SPI LCD, a capacitive touch panel, and streams sensor data over Bluetooth.

The project currently does five things:

- routes the Zephyr console and shell to the ESP32-S3 native USB serial/JTAG port
- initializes a `GC9A01` 240x240 round LCD over `SPI3`
- initializes a `CST816T`-style touch controller on `I2C`
- runs a minimal LVGL UI that shows `Hello!`, touch coordinates, and a touch indicator dot
- establishes a Bluetooth Low Energy (BLE) peripheral connection to send live sensor data to a smartphone


## Hardware

Target board:

- `esp32s3_devkitc/esp32s3/procpu`

Connected peripherals:

- LCD controller: `GC9A01`
- Touch controller: `CST816T` using Zephyr's `hynitron,cst816s` driver path
- BLE Radio: ESP32-S3 internal Bluetooth controller
- Sensors: Connected via I2C/SPI for external data collection

## Pin Mapping

**LCD:**

- `SPI3_SCK` -> `GPIO12`
- `SPI3_MOSI` -> `GPIO11`
- `SPI3_MISO` -> `GPIO13`
- `LCD_CS` -> `GPIO4`
- `LCD_DC` -> `GPIO5`
- `LCD_RST` -> `GPIO3`
- `LCD_BL` -> `GPIO7`
- `LCD power enable` -> `GPIO18`

**Touch:**

- `I2C_SDA` -> `GPIO8`
- `I2C_SCL` -> `GPIO9`
- `TP_INT` -> `GPIO1`
- `TP_RST` -> `GPIO2`

These connections are described in [app.overlay](app.overlay).

## What The App Does

On boot, the application:

1. enables the LCD backlight
2. initializes the display after deferred Zephyr device init
3. starts LVGL
4. creates a simple screen with a centered `Hello!` label
5. initializes the BLE stack and begins advertising as a connectable peripheral
6. listens for touch events from the touch controller
7. updates the UI with the current touch position
8. streams live sensor data to any connected smartphone via a custom GATT service

**When the panel is touched:**

- a small dot is drawn under the finger
- the bottom status label shows the transformed screen coordinates

**When the panel is released:**

- the dot is hidden
- the status text changes to `Touch released`

The display is rotated 180 degrees in devicetree so the UI matches the physical mounting orientation.

## Project Structure

- [CMakeLists.txt](CMakeLists.txt): Zephyr app definition and source list
- [prj.conf](prj.conf): Zephyr Kconfig options for console, shell, LVGL, display, input, **and Bluetooth (BLE/GATT)**
- [app.overlay](app.overlay): board-specific devicetree overlay with LCD and touch wiring
- [src/main.c](src/main.c): minimal entry point, including BLE advertising and sensor polling logic
- [src/panel.c](src/panel.c): LCD, touch, LVGL, and shell command logic
- [build.sh](build.sh): local build/flash helper script

## Building

The helper script expects a Zephyr workspace layout like this:

- this repository in its current directory
- Zephyr at `../zephyr`
- optional Python virtual environment at `../.venv`

Build:

```bash
./build.sh
```

Clean only:

```bash
./build.sh --clean
```

Pristine rebuild:

```bash
./build.sh --pristine
```

Override the default board if needed:

```bash
BOARD=esp32s3_devkitc/esp32s3/procpu ./build.sh
```

## Flashing

Flash the board:

```bash
./build.sh --flash --port /dev/cu.usbmodemXXXX
```

Optional erase before flashing:

```bash
./build.sh --flash --erase --port /dev/cu.usbmodemXXXX
```

After flashing, the script opens a serial terminal at `115200`.

## USB Console And Shell

The project routes both console and shell to the ESP32-S3 USB serial/JTAG peripheral.

Once connected, Zephyr shell commands are available together with one project-specific command:

```text
app
```

`app` prints the current display geometry, orientation, and the latest touch state.

## Notes

- The LCD panel is configured through Zephyr's `galaxycore,gc9x01x` driver.
- The touch controller is described as `hynitron,cst816s` in devicetree because that is the supported Zephyr driver path used by this hardware setup.
- The application is intentionally small and focused on hardware bring-up rather than product UI structure.
