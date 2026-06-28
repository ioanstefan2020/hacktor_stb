#ifndef BLE_H
#define BLE_H

#include <stdbool.h>

struct transit_data {
    char route[16];
    char direction[32];
    int minutes;
    bool valid;
};

int ble_init(void);

bool ble_get_transit_update(struct transit_data *data);

#endif
