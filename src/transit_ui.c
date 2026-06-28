#include "transit_ui.h"
#include <stdio.h>
#include <lvgl.h>

#define COL_BG       0x000000
#define COL_ACCENT   0x00FF00
#define COL_TEXT     0xFFFFFF

static lv_obj_t *route_label;
static lv_obj_t *dir_label;
static lv_obj_t *time_label;
static lv_obj_t *status_label;

void transit_ui_create(void)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(COL_BG), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    /* Bus Line Number */
    route_label = lv_label_create(screen);
    lv_obj_set_style_text_color(route_label, lv_color_hex(COL_ACCENT), 0);
    lv_obj_set_style_text_font(route_label, &lv_font_montserrat_28, 0);
    lv_obj_align(route_label, LV_ALIGN_CENTER, 0, -40);
    lv_label_set_text(route_label, "--");

    /* Destination/Direction */
    dir_label = lv_label_create(screen);
    lv_obj_set_style_text_color(dir_label, lv_color_hex(COL_TEXT), 0);
    lv_obj_set_style_text_font(dir_label, &lv_font_montserrat_14, 0);
    lv_obj_align(dir_label, LV_ALIGN_CENTER, 0, -10);
    lv_label_set_text(dir_label, "Waiting for STB Data...");

    /* Minutes until arrival */
    time_label = lv_label_create(screen);
    lv_obj_set_style_text_color(time_label, lv_color_hex(COL_TEXT), 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_28, 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 30);
    lv_label_set_text(time_label, "");

    /* Connection Status */
    status_label = lv_label_create(screen);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_label_set_text(status_label, "BLE Disconnected");
}

void transit_ui_update(const struct transit_data *data)
{
    if (!data->valid) return;

    char buf[32];

    lv_label_set_text(route_label, data->route);
    lv_label_set_text(dir_label, data->direction);

    if (data->minutes == 0) {
        lv_label_set_text(time_label, "Arriving Now!");
    } else {
        snprintf(buf, sizeof(buf), "%d min", data->minutes);
        lv_label_set_text(time_label, buf);
    }

    lv_label_set_text(status_label, "Sync Active");
}
