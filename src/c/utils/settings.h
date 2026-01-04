#pragma once

#include <pebble.h>

#define SETTINGS_KEY 1

typedef struct LocalSettings{
    uint32_t favorites;  // on/off for each of up to 32 favorites
} LocalSettings;

static LocalSettings settings;

LocalSettings* settings_get();
void settings_load_default();
void settings_save();
void settings_load();