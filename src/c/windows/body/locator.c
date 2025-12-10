#include "locator.h"
#include "../../providers/altitude_provider.h"
#include "../../providers/azimuth_provider.h"
#include "../../style.h"

static Window *s_window;
static Layer *s_crosshair_layer;
static TextLayer *s_title_layer;
static TextLayer *s_target_layer;
static TextLayer *s_current_layer;
static StatusBarLayer *s_status_layer;

static TargetData s_target = {42, 245};
static int16_t s_current_altitude_deg = 0;
static int16_t s_current_azimuth_deg = 0;

static int16_t prv_normalize_azimuth_delta(int16_t delta) {
  // Wrap into [-180, 180] for smallest rotation distance.
  while (delta > 180) {
    delta -= 360;
  }
  while (delta < -180) {
    delta += 360;
  }
  return delta;
}

static void prv_on_altitude(int16_t altitude_deg) {
  locator_set_current_altitude(altitude_deg);
}

static void prv_on_azimuth(int16_t azimuth_deg) {
  locator_set_current_azimuth(azimuth_deg);
}

static void prv_update_labels(void) {
  if (!s_target_layer || !s_current_layer) {
    return;
  }

  static char s_target_text[32];
  static char s_current_text[32];
  snprintf(s_target_text, sizeof(s_target_text), "Target Alt %d° | Az %d°",
           s_target.altitude_deg, s_target.azimuth_deg);
  snprintf(s_current_text, sizeof(s_current_text), "My Alt %d° | Az %d°",
           s_current_altitude_deg, s_current_azimuth_deg);
  text_layer_set_text(s_target_layer, s_target_text);
  text_layer_set_text(s_current_layer, s_current_text);
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

  // Target indicator: offset from center by current vs. target deltas.
  const int16_t delta_alt = s_target.altitude_deg - s_current_altitude_deg;
  const int16_t delta_az = prv_normalize_azimuth_delta(s_target.azimuth_deg - s_current_azimuth_deg);

  // Map degrees to pixels using the crosshair radius; clamp to stay on the reticle.
  const int16_t max_span_deg = 90;  // map +/-90° to full radius
  int16_t dx = (int16_t)((radius * delta_az) / max_span_deg);
  int16_t dy = (int16_t)((-radius * delta_alt) / max_span_deg);  // negative to move up for positive altitude
  if (dx > (int16_t)radius) dx = radius;
  if (dx < -(int16_t)radius) dx = -(int16_t)radius;
  if (dy > (int16_t)radius) dy = radius;
  if (dy < -(int16_t)radius) dy = -(int16_t)radius;

  const uint16_t target_radius = radius / 6 > 2 ? radius / 6 : 2;
  GPoint target_center = GPoint(center.x + dx, center.y + dy);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, target_center, target_radius);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_circle(ctx, target_center, target_radius);
}

static void prv_window_load(Window *window) {
  const Layout *layout = layout_get();
  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  s_status_layer = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_layer, layout->background, layout->foreground);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_layer));

  const GRect content_bounds =
      GRect(bounds.origin.x, bounds.origin.y + STATUS_BAR_LAYER_HEIGHT,
           bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT);

  s_crosshair_layer = layer_create(content_bounds);
  layer_set_update_proc(s_crosshair_layer, prv_draw_crosshair);
  layer_add_child(window_layer, s_crosshair_layer);

  s_title_layer = text_layer_create(GRect(0, 8, content_bounds.size.w, 24));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_text(s_title_layer, "Targeter");
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  s_target_layer = text_layer_create(GRect(0, content_bounds.size.h - 64, content_bounds.size.w, 20));
  text_layer_set_text_alignment(s_target_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_target_layer, GColorWhite);
  text_layer_set_background_color(s_target_layer, GColorClear);
  text_layer_set_font(s_target_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_target_layer));

  s_current_layer = text_layer_create(GRect(0, content_bounds.size.h - 34, content_bounds.size.w, 20));
  text_layer_set_text_alignment(s_current_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_current_layer, GColorWhite);
  text_layer_set_background_color(s_current_layer, GColorClear);
  text_layer_set_font(s_current_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(s_current_layer));

  prv_update_labels();
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_target_layer);
  text_layer_destroy(s_current_layer);
  layer_destroy(s_crosshair_layer);
  status_bar_layer_destroy(s_status_layer);
}

void locator_init(void) {
  if (s_window) {
    return;
  }

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers){
                                    .load = prv_window_load,
                                    .unload = prv_window_unload,
                                });

  // Start sensors/providers after window creation so callbacks can update labels.
  altitude_provider_init();
  altitude_provider_set_handler(prv_on_altitude);

  azimuth_provider_init();
  azimuth_provider_set_handler(prv_on_azimuth);
}

void locator_deinit(void) {
  if (!s_window) {
    return;
  }

  azimuth_provider_deinit();
  altitude_provider_deinit();

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
  s_crosshair_layer = NULL;
  s_title_layer = NULL;
  s_target_layer = NULL;
  s_current_layer = NULL;
  s_status_layer = NULL;
}

void locator_set_target(int16_t altitude_deg, int16_t azimuth_deg) {
  s_target.altitude_deg = altitude_deg;
  s_target.azimuth_deg = azimuth_deg;
  prv_update_labels();
}

TargetData locator_get_target(void) { return s_target; }

void locator_set_current_altitude(int16_t altitude_deg) {
  s_current_altitude_deg = altitude_deg;
  prv_update_labels();
}

int16_t locator_get_current_altitude(void) { return s_current_altitude_deg; }

void locator_set_current_azimuth(int16_t azimuth_deg) {
  s_current_azimuth_deg = azimuth_deg;
  prv_update_labels();
}

int16_t locator_get_current_azimuth(void) { return s_current_azimuth_deg; }

void locator_show(void) {
  if (!s_window) {
    locator_init();
  }
  window_stack_push(s_window, true);
}

void locator_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
  }
}

