#pragma once

#include <pebble.h>

typedef void (*AltitudeUpdateHandler)(int16_t altitude_deg);

void altitude_provider_init(void);
void altitude_provider_deinit(void);

void altitude_provider_set_handler(AltitudeUpdateHandler handler);
int16_t altitude_provider_get_altitude_deg(void);

