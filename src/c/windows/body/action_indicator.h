#pragma once

#include <pebble.h>

// Lifecycle functions
Layer *action_indicator_create(GRect bounds);
void action_indicator_add_to_window(Window *window);
void action_indicator_destroy(void);

// Visibility control
void action_indicator_set_visible(bool visible);

// Drawing functions
void action_indicator_draw(GContext *ctx, Layer *layer);
void action_indicator_update_proc(Layer *action_indicator_layer, GContext *ctx);