#include "style.h"

static Layout s_layout;
static bool s_layout_initialized;

const Layout *layout_get(void) {
  if (!s_layout_initialized) {
    s_layout.background = GColorBlack;
    s_layout.foreground = GColorWhite;
    s_layout.highlight = PBL_IF_COLOR_ELSE(GColorLavenderIndigo, GColorWhite);
    s_layout.highlight_foreground = PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack);
    s_layout_initialized = true;
  }
  return &s_layout;
}
