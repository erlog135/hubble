#pragma once

#include <pebble.h>

typedef struct {
  int16_t altitude_deg;
  int16_t azimuth_deg;
} TargetData;

void locator_init(void);
void locator_deinit(void);

void locator_set_target(int16_t altitude_deg, int16_t azimuth_deg);
TargetData locator_get_target(void);

void locator_set_current_altitude(int16_t altitude_deg);
int16_t locator_get_current_altitude(void);
void locator_set_current_azimuth(int16_t azimuth_deg);
int16_t locator_get_current_azimuth(void);

void locator_show(void);
void locator_hide(void);

