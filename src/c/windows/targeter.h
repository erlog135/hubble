#pragma once

#include <pebble.h>

typedef struct {
  int16_t altitude_deg;
  int16_t azimuth_deg;
} TargetData;

void targeter_init(void);
void targeter_deinit(void);

void targeter_set_target(int16_t altitude_deg, int16_t azimuth_deg);
TargetData targeter_get_target(void);

void targeter_set_current_altitude(int16_t altitude_deg);
int16_t targeter_get_current_altitude(void);

void targeter_show(void);
void targeter_hide(void);

