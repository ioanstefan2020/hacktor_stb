#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <lvgl.h>
#include <lvgl_zephyr.h>

#include "display.h"
#include "ble.h"
#include "transit_ui.h"

int main(void)
{
    printk("Hacktor_Watch Starting...\n");

    /* Initialize Subsystems */
    if (display_init() < 0) {
        printk("Display init failed\n");
        return -1;
    }

    ble_init();

    /* Setup UI */
    lvgl_lock();
    transit_ui_create();
    lv_timer_handler();
    lvgl_unlock();

    struct transit_data incoming_data;

    while (1) {
        if (ble_get_transit_update(&incoming_data)) {
            lvgl_lock();
            transit_ui_update(&incoming_data);
            lvgl_unlock();
        }

        lvgl_lock();
        uint32_t sleep_ms = lv_timer_handler();
        lvgl_unlock();

        if (sleep_ms < 10) sleep_ms = 10;
        k_sleep(K_MSEC(sleep_ms));
    }
}
