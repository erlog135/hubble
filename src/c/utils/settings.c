#include "settings.h"


LocalSettings* settings_get() {
    return &settings;
}

void settings_save() {
    persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void settings_load_default() {
    settings.favorites = 0; // all off
    settings.magnetic_declination = 0;
}

void settings_load() {
    settings_load_default();

    if(!persist_exists(SETTINGS_KEY)) return;

    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}