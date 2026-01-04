#pragma once

#include <pebble.h>

// Initialize the action result window
void action_result_init(void);

// Deinitialize the action result window
void action_result_deinit(void);

// Show the action result window with a message
void action_result_show(const char *message);

// Hide the action result window
void action_result_hide(void);
