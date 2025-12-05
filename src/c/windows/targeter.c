#include "targeter.h"

static Window *s_window;
static Layer *s_crosshair_layer;
static TextLayer *s_title_layer;
static TextLayer *s_alt_layer;
static TextLayer *s_current_alt_layer;
static TextLayer *s_az_layer;

static TargetData s_target = {0, 0};
static int16_t s_current_altitude_deg = 0;

static void prv_update_labels(void) {
  if (!s_alt_layer || !s_az_layer || !s_current_alt_layer) {
    return;
  }

  static char s_target_alt_text[24];
  static char s_current_alt_text[24];
  static char s_az_text[24];
  snprintf(s_target_alt_text, sizeof(s_target_alt_text), "Target Alt: %d°", s_target.altitude_deg);
  snprintf(s_current_alt_text, sizeof(s_current_alt_text), "Current Alt: %d°", s_current_altitude_deg);
  snprintf(s_az_text, sizeof(s_az_text), "Target Az: %d°", s_target.azimuth_deg);
  text_layer_set_text(s_alt_layer, s_target_alt_text);
  text_layer_set_text(s_current_alt_layer, s_current_alt_text);
  text_layer_set_text(s_az_layer, s_az_text);
}

static void prv_draw_crosshair(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);
  const GPoint center = grect_center_point(&bounds);
  const uint16_t radius = bounds.size.w < bounds.size.h ? bounds.size.w / 4 : bounds.size.h / 4;

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Concentric circles
  graphics_draw_circle(ctx, center, radius);
  graphics_draw_circle(ctx, center, radius / 2);

  // Crosshair lines
  graphics_draw_line(ctx, GPoint(center.x - radius, center.y), GPoint(center.x + radius, center.y));
  graphics_draw_line(ctx, GPoint(center.x, center.y - radius), GPoint(center.x, center.y + radius));

  // Small center dot
  graphics_fill_circle(ctx, center, 2);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  s_crosshair_layer = layer_create(bounds);
  layer_set_update_proc(s_crosshair_layer, prv_draw_crosshair);
  layer_add_child(window_layer, s_crosshair_layer);

  s_title_layer = text_layer_create(GRect(0, 8, bounds.size.w, 24));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_text(s_title_layer, "Targeter");
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  s_alt_layer = text_layer_create(GRect(0, bounds.size.h - 64, bounds.size.w, 20));
  text_layer_set_text_alignment(s_alt_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_alt_layer, GColorWhite);
  text_layer_set_background_color(s_alt_layer, GColorClear);
  text_layer_set_font(s_alt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(s_alt_layer));

  s_current_alt_layer = text_layer_create(GRect(0, bounds.size.h - 44, bounds.size.w, 20));
  text_layer_set_text_alignment(s_current_alt_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_current_alt_layer, GColorWhite);
  text_layer_set_background_color(s_current_alt_layer, GColorClear);
  text_layer_set_font(s_current_alt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(s_current_alt_layer));

  s_az_layer = text_layer_create(GRect(0, bounds.size.h - 24, bounds.size.w, 20));
  text_layer_set_text_alignment(s_az_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_az_layer, GColorWhite);
  text_layer_set_background_color(s_az_layer, GColorClear);
  text_layer_set_font(s_az_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(s_az_layer));

  prv_update_labels();
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_alt_layer);
  text_layer_destroy(s_current_alt_layer);
  text_layer_destroy(s_az_layer);
  layer_destroy(s_crosshair_layer);
}

void targeter_init(void) {
  if (s_window) {
    return;
  }

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers){
                                    .load = prv_window_load,
                                    .unload = prv_window_unload,
                                });
}

void targeter_deinit(void) {
  if (!s_window) {
    return;
  }

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
  s_crosshair_layer = NULL;
  s_title_layer = NULL;
  s_alt_layer = NULL;
  s_az_layer = NULL;
}

void targeter_set_target(int16_t altitude_deg, int16_t azimuth_deg) {
  s_target.altitude_deg = altitude_deg;
  s_target.azimuth_deg = azimuth_deg;
  prv_update_labels();
}

TargetData targeter_get_target(void) { return s_target; }

void targeter_set_current_altitude(int16_t altitude_deg) {
  s_current_altitude_deg = altitude_deg;
  prv_update_labels();
}

int16_t targeter_get_current_altitude(void) { return s_current_altitude_deg; }

void targeter_show(void) {
  if (!s_window) {
    targeter_init();
  }
  window_stack_push(s_window, true);
}

void targeter_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
  }
}

