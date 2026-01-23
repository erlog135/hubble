#include "action_indicator.h"


static Layer *s_action_indicator_layer;
static bool s_action_indicator_visible;

Layer *action_indicator_create(GRect bounds) {
  // Create the layer
  s_action_indicator_layer = layer_create(bounds);

  // Set the update procedure
  layer_set_update_proc(s_action_indicator_layer, action_indicator_update_proc);

  return s_action_indicator_layer;
}

void action_indicator_add_to_window(Window *window) {
  if (s_action_indicator_layer) {
    layer_add_child(window_get_root_layer(window), s_action_indicator_layer);
  }
}

void action_indicator_set_visible(bool visible) {
  s_action_indicator_visible = visible;
  if (s_action_indicator_layer) {
    layer_set_hidden(s_action_indicator_layer, !visible);
  }
}

void action_indicator_destroy(void) {
  if (s_action_indicator_layer) {
    layer_destroy(s_action_indicator_layer);
    s_action_indicator_layer = NULL;
  }
}

//done like how the system does but with accessible functions
void action_indicator_draw(GContext *ctx, Layer *layer) {
  // Don't draw if not visible
  if (!s_action_indicator_visible) {
    return;
  }

  // Get bounds from the window's root layer
  Window *window = layer_get_window(layer);
  Layer *root_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(root_layer);

  // Calculate circle center point and radius directly
  const int radius = PBL_IF_ROUND_ELSE(12, 13);

  // Calculate offset based on preferred content size
  int offset = 0;
  switch (preferred_content_size()) {
    case PreferredContentSizeSmall:
      offset = PBL_IF_ROUND_ELSE(1, 8);
      break;
    case PreferredContentSizeMedium:
      offset = PBL_IF_ROUND_ELSE(1, 8);
      break;
    case PreferredContentSizeLarge:
      offset = 4;
      break;
    case PreferredContentSizeExtraLarge:
      offset = 4;
      break;
    default:
      offset = PBL_IF_ROUND_ELSE(1, 8);
      break;
  }

  // Center point: positioned off right edge with edge visible, centered vertically
  GPoint center = GPoint(
    bounds.origin.x + bounds.size.w + offset,  // Right edge + offset
    bounds.origin.y + bounds.size.h / 2        // Vertical center
  );

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, radius);
}

void action_indicator_update_proc(Layer *action_indicator_layer, GContext *ctx) {
  action_indicator_draw(ctx, action_indicator_layer);
}