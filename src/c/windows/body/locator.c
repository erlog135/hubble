#include "locator.h"
#include "../../providers/altitude_provider.h"
#include "../../providers/azimuth_provider.h"
#include "../../style.h"
#include <string.h>

#define GRID_MARGIN 0
#define GRID_ROWS 2
#define GRID_COLS 2
#define GRID_ROW_HEIGHT 22

static Window *s_window;
static Layer *s_crosshair_layer;
static TextLayer *s_target_grid[GRID_ROWS][GRID_COLS];
static TextLayer *s_current_grid[GRID_ROWS][GRID_COLS];
static StatusBarLayer *s_status_layer;
static ActionBarLayer *s_action_bar;
static GBitmap *s_icon_light_on;
static GBitmap *s_icon_light_off;
static GBitmap *s_icon_vibe_on;
static GBitmap *s_icon_vibe_off;
static bool s_light_enabled;
static bool s_vibe_enabled;

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
  if (!s_target_grid[1][0] || !s_target_grid[1][1] || !s_current_grid[1][0] ||
      !s_current_grid[1][1]) {
    return;
  }

  static char s_target_alt_text[8];
  static char s_target_az_text[8];
  static char s_current_alt_text[8];
  static char s_current_az_text[8];

  snprintf(s_target_alt_text, sizeof(s_target_alt_text), "%d°", s_target.altitude_deg);
  snprintf(s_target_az_text, sizeof(s_target_az_text), "%d°", s_target.azimuth_deg);
  snprintf(s_current_alt_text, sizeof(s_current_alt_text), "%d°", s_current_altitude_deg);
  snprintf(s_current_az_text, sizeof(s_current_az_text), "%d°", s_current_azimuth_deg);

  text_layer_set_text(s_target_grid[1][0], s_target_alt_text);
  text_layer_set_text(s_target_grid[1][1], s_target_az_text);
  text_layer_set_text(s_current_grid[1][0], s_current_alt_text);
  text_layer_set_text(s_current_grid[1][1], s_current_az_text);
}

static void prv_update_action_icons(void) {
  if (!s_action_bar) {
    return;
  }
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP,
                            s_light_enabled ? s_icon_light_on : s_icon_light_off);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN,
                            s_vibe_enabled ? s_icon_vibe_on : s_icon_vibe_off);
}

static void prv_light_toggle_click_handler(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  s_light_enabled = !s_light_enabled;
  prv_update_action_icons();
}

static void prv_vibe_toggle_click_handler(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  s_vibe_enabled = !s_vibe_enabled;
  prv_update_action_icons();
}

static void prv_click_config_provider(void *context) {
  (void)context;
  window_single_click_subscribe(BUTTON_ID_UP, prv_light_toggle_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_vibe_toggle_click_handler);
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

static void prv_create_grid(TextLayer *grid[GRID_ROWS][GRID_COLS], Layer *parent,
                            GRect bounds, const char *text[GRID_ROWS][GRID_COLS],
                            GFont header_font, GFont value_font, const GColor color) {
  const int16_t column_width = (bounds.size.w - GRID_MARGIN * 3) / 2;
  int16_t y = bounds.origin.y;

  for (int row = 0; row < GRID_ROWS; ++row) {
    int16_t x = bounds.origin.x + GRID_MARGIN;
    for (int col = 0; col < GRID_COLS; ++col) {
      GRect frame = GRect(x, y, column_width, GRID_ROW_HEIGHT);
      grid[row][col] = text_layer_create(frame);
      text_layer_set_text(grid[row][col], text[row][col]);
      text_layer_set_background_color(grid[row][col], GColorClear);
      text_layer_set_text_color(grid[row][col], color);
      text_layer_set_font(grid[row][col], row == 0 ? header_font : value_font);
      text_layer_set_text_alignment(grid[row][col], GTextAlignmentCenter);
      text_layer_set_overflow_mode(grid[row][col], GTextOverflowModeWordWrap);
      layer_add_child(parent, text_layer_get_layer(grid[row][col]));

      x += column_width + GRID_MARGIN;
    }
    y += GRID_ROW_HEIGHT + GRID_MARGIN;
  }
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
           bounds.size.w - ACTION_BAR_WIDTH, bounds.size.h - STATUS_BAR_LAYER_HEIGHT);

  const GFont header_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  const GFont value_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);

  const int16_t grid_height = GRID_ROW_HEIGHT * GRID_ROWS + GRID_MARGIN;
  const int16_t top_grid_y = content_bounds.origin.y + GRID_MARGIN;
  const int16_t bottom_grid_y =
      content_bounds.origin.y + content_bounds.size.h - grid_height - GRID_MARGIN;

  // Crosshair uses full content bounds so its scaling ignores text layout.
  s_crosshair_layer = layer_create(content_bounds);
  layer_set_update_proc(s_crosshair_layer, prv_draw_crosshair);
  layer_add_child(window_layer, s_crosshair_layer);

  const char *target_text[GRID_ROWS][GRID_COLS] = {
      {"Alt", "Az"},
      {"", ""},
  };
  const GRect target_grid_frame =
      GRect(content_bounds.origin.x, top_grid_y, content_bounds.size.w, grid_height);
  prv_create_grid(s_target_grid, window_layer, target_grid_frame, target_text, header_font,
                  value_font, layout->foreground);

  const char *current_text[GRID_ROWS][GRID_COLS] = {
      {"My Alt", "My Az"},
      {"", ""},
  };
  const GRect current_grid_frame =
      GRect(content_bounds.origin.x, bottom_grid_y, content_bounds.size.w, grid_height);
  prv_create_grid(s_current_grid, window_layer, current_grid_frame, current_text, header_font,
                  value_font, layout->foreground);

  // Action bar on the right edge.
  s_light_enabled = false;
  s_vibe_enabled = false;
  s_icon_light_on = gbitmap_create_with_resource(RESOURCE_ID_ACTION_LIGHT_ON);
  s_icon_light_off = gbitmap_create_with_resource(RESOURCE_ID_ACTION_LIGHT_OFF);
  s_icon_vibe_on = gbitmap_create_with_resource(RESOURCE_ID_ACTION_VIBRATE_ENABLE);
  s_icon_vibe_off = gbitmap_create_with_resource(RESOURCE_ID_ACTION_VIBRATE_DISABLE);

  s_action_bar = action_bar_layer_create();
  action_bar_layer_set_click_config_provider(s_action_bar, prv_click_config_provider);
  action_bar_layer_set_background_color(s_action_bar, PBL_IF_COLOR_ELSE(GColorImperialPurple, GColorBlack));
  action_bar_layer_add_to_window(s_action_bar, s_window);
  prv_update_action_icons();

  prv_update_labels();
}

static void prv_window_unload(Window *window) {
  for (int row = 0; row < GRID_ROWS; ++row) {
    for (int col = 0; col < GRID_COLS; ++col) {
      if (s_target_grid[row][col]) {
        text_layer_destroy(s_target_grid[row][col]);
        s_target_grid[row][col] = NULL;
      }
      if (s_current_grid[row][col]) {
        text_layer_destroy(s_current_grid[row][col]);
        s_current_grid[row][col] = NULL;
      }
    }
  }
  if (s_action_bar) {
    action_bar_layer_remove_from_window(s_action_bar);
    action_bar_layer_destroy(s_action_bar);
    s_action_bar = NULL;
  }
  if (s_icon_light_on) {
    gbitmap_destroy(s_icon_light_on);
    s_icon_light_on = NULL;
  }
  if (s_icon_light_off) {
    gbitmap_destroy(s_icon_light_off);
    s_icon_light_off = NULL;
  }
  if (s_icon_vibe_on) {
    gbitmap_destroy(s_icon_vibe_on);
    s_icon_vibe_on = NULL;
  }
  if (s_icon_vibe_off) {
    gbitmap_destroy(s_icon_vibe_off);
    s_icon_vibe_off = NULL;
  }
  layer_destroy(s_crosshair_layer);
  s_crosshair_layer = NULL;
  status_bar_layer_destroy(s_status_layer);
  s_status_layer = NULL;
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
  memset(s_target_grid, 0, sizeof(s_target_grid));
  memset(s_current_grid, 0, sizeof(s_current_grid));
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

