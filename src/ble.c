#include "ble.h"
#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define BT_UUID_STB_SVC \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0))

#define BT_UUID_TRANSIT_CHAR \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef3))

static struct transit_data current_transit;
static bool transit_updated = false;
static K_MUTEX_DEFINE(transit_mutex);

static ssize_t transit_write_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    char str[64];
    if (len >= sizeof(str)) {
        len = sizeof(str) - 1;
    }
    memcpy(str, buf, len);
    str[len] = '\0';

    char route[16] = {0};
    char dir[32] = {0};
    int mins = 0;

    if (sscanf(str, "%15[^,],%31[^,],%d", route, dir, &mins) == 3) {
        k_mutex_lock(&transit_mutex, K_FOREVER);
        strncpy(current_transit.route, route, sizeof(current_transit.route));
        strncpy(current_transit.direction, dir, sizeof(current_transit.direction));
        current_transit.minutes = mins;
        current_transit.valid = true;
        transit_updated = true;
        k_mutex_unlock(&transit_mutex);

        printk("STB Update: %s towards %s in %d min\n", route, dir, mins);
    } else {
        printk("Failed to parse STB payload: %s\n", str);
    }
    return len;
}

BT_GATT_SERVICE_DEFINE(stb_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_STB_SVC),
    BT_GATT_CHARACTERISTIC(BT_UUID_TRANSIT_CHAR,
                           BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                           BT_GATT_PERM_WRITE, NULL, transit_write_cb, NULL),
);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, "Hacktor_Watch", 11),
};

int ble_init(void)
{
    int err = bt_enable(NULL);
    if (err) return err;

    err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) return err;

    printk("BLE ready\n");
    return 0;
}

bool ble_get_transit_update(struct transit_data *data)
{
    bool updated = false;
    k_mutex_lock(&transit_mutex, K_FOREVER);
    if (transit_updated) {
        *data = current_transit;
        transit_updated = false;
        updated = true;
    }
    k_mutex_unlock(&transit_mutex);
    return updated;
}
