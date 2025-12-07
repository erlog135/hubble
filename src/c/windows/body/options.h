#pragma once

#include <pebble.h>

// Opens the options action menu with Favorite, Locate, Refresh, Hide.
void options_menu_show(void);

// Cleans up any active menu (safe to call even if not shown).
void options_menu_deinit(void);