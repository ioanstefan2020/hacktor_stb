#include "panel.h"

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>
#include <lvgl.h>

/* lvgl_lock/unlock are not available in this Zephyr version. */
#define lvgl_lock()
#define lvgl_unlock()

#define DISPLAY_NODE DT_CHOSEN(zephyr_display)
#define BACKLIGHT_NODE DT_NODELABEL(lcd_backlight)
#define DISPLAY_WIDTH DT_PROP(DISPLAY_NODE, width)
#define DISPLAY_HEIGHT DT_PROP(DISPLAY_NODE, height)

static const struct gpio_dt_spec lcd_backlight = GPIO_DT_SPEC_GET(BACKLIGHT_NODE, gpios);

static struct display_capabilities display_cap = {
    .x_resolution = DISPLAY_WIDTH,
    .y_resolution = DISPLAY_HEIGHT,
    .current_orientation = DISPLAY_ORIENTATION_NORMAL,
};

static lv_obj_t *status_label;

static int cmd_app_status(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "display: %ux%u orientation=%u",
            display_cap.x_resolution,
            display_cap.y_resolution,
            display_cap.current_orientation);

    return 0;
}
SHELL_CMD_REGISTER(app, NULL, "Show current app status", cmd_app_status);

static int init_display(const struct device **display_dev)
{
    const struct device *display = DEVICE_DT_GET(DISPLAY_NODE);
    int ret;

    if (!gpio_is_ready_dt(&lcd_backlight)) {
        printk("LCD backlight GPIO is not ready\n");
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&lcd_backlight, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Backlight GPIO setup failed: %d\n", ret);
        return ret;
    }

    if (!device_is_ready(display)) {
        printk("Display device is not ready\n");
        return -ENODEV;
    }

    display_get_capabilities(display, &display_cap);

    /* Turn off display blanking so the UI is visible */
    display_blanking_off(display);

    *display_dev = display;
    return 0;
}

static int init_ui(void)
{
    lv_obj_t *screen;
    lv_obj_t *hello_label;

    screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    hello_label = lv_label_create(screen);
    lv_label_set_text(hello_label, "Hello!");
    lv_obj_set_style_text_color(hello_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(hello_label, &lv_font_montserrat_42, 0);
    lv_obj_align(hello_label, LV_ALIGN_CENTER, 0, 0);

    status_label = lv_label_create(screen);
    lv_label_set_text(status_label, "Hacktor Watch");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x8CFFB5), 0);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -14);

    return 0;
}

int panel_run(void)
{
    const struct device *display;
    uint32_t sleep_ms;
    int ret;

    ret = init_display(&display);
    if (ret < 0) {
        return 0;
    }

    lvgl_lock();
    ret = init_ui();
    if (ret == 0) {
        lv_timer_handler();
    }
    lvgl_unlock();

    if (ret < 0) {
        return 0;
    }

    while (1) {
        lvgl_lock();
        sleep_ms = lv_timer_handler();
        lvgl_unlock();

        if (sleep_ms == LV_NO_TIMER_READY || sleep_ms > 100U) {
            sleep_ms = 100U;
        }
        if (sleep_ms < 10U) {
            sleep_ms = 10U;
        }

        k_sleep(K_MSEC(sleep_ms));
    }
}
