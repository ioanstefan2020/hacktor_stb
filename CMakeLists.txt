cmake_minimum_required(VERSION 3.20.0)

set(BOARD esp32s3_devkitm CACHE STRING "Default Zephyr board")
set(DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hacktor_stb_watch)

target_sources(app PRIVATE
    src/main.c
    src/display.c
    src/transit_ui.c
    src/ble.c
)
