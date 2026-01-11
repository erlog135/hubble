#pragma once

#include <pebble.h>

typedef void (*AzimuthUpdateHandler)(int16_t azimuth_deg);
typedef void (*CalibrationUpdateHandler)(bool is_calibrated);

void azimuth_provider_init(void);
void azimuth_provider_deinit(void);

void azimuth_provider_set_handler(AzimuthUpdateHandler handler);
void azimuth_provider_set_calibration_handler(CalibrationUpdateHandler handler);
int16_t azimuth_provider_get_azimuth_deg(void);
bool azimuth_provider_is_calibrated(void);

