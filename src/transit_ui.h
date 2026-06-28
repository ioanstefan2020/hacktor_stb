#ifndef TRANSIT_UI_H
#define TRANSIT_UI_H

#include "ble.h"

void transit_ui_create(void);

void transit_ui_update(const struct transit_data *data);

#endif
