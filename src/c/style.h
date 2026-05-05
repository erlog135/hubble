#pragma once

#include <pebble.h>

#define LAYOUT_BACKGROUND GColorBlack
#define LAYOUT_FOREGROUND GColorWhite
#define LAYOUT_HIGHLIGHT PBL_IF_COLOR_ELSE(GColorVividViolet, GColorWhite)
#define LAYOUT_HIGHLIGHT_FG PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack)

// Pebble system fonts have an implicit top glyph offset inside layer bounds.
#define FONT_GLYPH_TOP_OFFSET 3
