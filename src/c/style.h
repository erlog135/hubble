#pragma once

#include <pebble.h>

typedef struct {
  GColor background;
  GColor foreground;
  GColor highlight;
  GColor highlight_foreground;
} Layout;

// Returns app-wide layout colors. Always returns the same instance.
const Layout *layout_get(void);
