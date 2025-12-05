#pragma once

#include <pebble.h>

typedef void (*AzimuthUpdateHandler)(int16_t azimuth_deg);

void azimuth_provider_init(void);
void azimuth_provider_deinit(void);

void azimuth_provider_set_handler(AzimuthUpdateHandler handler);
int16_t azimuth_provider_get_azimuth_deg(void);

