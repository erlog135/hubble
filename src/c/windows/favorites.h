#pragma once

#include <pebble.h>

void favorites_init(void);
void favorites_deinit(void);

void favorites_show(void);
void favorites_hide(void);

// Get the current favorites window (NULL if not initialized)
Window* favorites_get_window(void);