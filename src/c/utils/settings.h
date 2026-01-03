#pragma once

#include <pebble.h>

#define SETTINGS_KEY 1

typedef struct ClaySettings{
    uint32_t favorites;  // on/off for each of up to 32 favorites
} ClaySettings;

static ClaySettings settings;

ClaySettings* settings_get();
void settings_load_default();
void settings_save();
void settings_load();