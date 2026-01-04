#include "details.h"
#include "../../style.h"
#include "../../utils/bodymsg.h"
#include "options.h"

#define HERO_IMAGE_SIZE 50
#define GRID_MARGIN 4
#define GRID_ROWS 2
#define GRID_COLS 2
#define GRID_ROW_HEIGHT 18
#define TITLE_TOP_MARGIN 2
#define TITLE_BOTTOM_MARGIN 4
#define HERO_IMAGE_FRAME_PADDING 6
#define HERO_IMAGE_BOTTOM_MARGIN 6
#define DETAIL_BOTTOM_MARGIN 6
#define LONG_TEXT_TOP_MARGIN 8

static Window *s_window;
static ScrollLayer *s_scroll_layer;
static Layer *s_image_layer;
static TextLayer *s_title_layer;
static TextLayer *s_detail_layer;
static TextLayer *s_grid_layers[GRID_ROWS][GRID_COLS];
static TextLayer *s_long_text_layer;
static GDrawCommandImage *s_pdc_image;
static GBitmap *s_bitmap_image;
static StatusBarLayer *s_status_layer;

static int16_t s_page_height;
static DetailsContent s_content;
static bool s_is_loading;

static bool prv_is_loading(void) {
  return s_is_loading;
}

static const DetailsContent s_default_content = {
    .title_text = "Moon",
    .detail_text = "Waning Crescent",
    .grid_top_left = "RISE",
    .grid_top_right = "SET",
    .grid_bottom_left = "8:00 PM",
    .grid_bottom_right = "11:30 AM",
    .long_text = "",  // Empty initially
    .image_resource_id = RESOURCE_ID_FULL_MOON,
    .image_type = DETAILS_IMAGE_TYPE_BITMAP,
    .azimuth_deg = 0,
    .altitude_deg = 0,
    .illumination_x10 = 0,
    .body_id = 0,  // Moon
};

static const DetailsContent s_loading_content = {
    .title_text = "Loading...",
    .detail_text = "Fetching data",
    .grid_top_left = "RISE",
    .grid_top_right = "SET",
    .grid_bottom_left = "--:--",
    .grid_bottom_right = "--:--",
    .long_text = "Loading...\nLoading...",
    .image_resource_id = RESOURCE_ID_FULL_MOON,  // Keep default image for now
    .image_type = DETAILS_IMAGE_TYPE_BITMAP,
    .azimuth_deg = 0,
    .altitude_deg = 0,
    .illumination_x10 = 0,
    .body_id = -1,  // Not a specific body
};

static GSize prv_get_image_size(void) {
  return GSize(HERO_IMAGE_SIZE, HERO_IMAGE_SIZE);
}

static GSize prv_calc_text_size(const char *text, GFont font, GRect frame) {
  return graphics_text_layout_get_content_size(text, font, frame,
                                               GTextOverflowModeWordWrap,
                                               GTextAlignmentLeft);
}

static void prv_draw_image(Layer *layer, GContext *ctx) {
  const GSize image_bounds = prv_get_image_size();
  if (image_bounds.w == 0 || image_bounds.h == 0) {
    return;
  }

  const GRect bounds = layer_get_bounds(layer);
  const GPoint origin = GPoint((bounds.size.w - image_bounds.w) / 2,
                               (bounds.size.h - image_bounds.h) / 2);

  if (s_content.image_type == DETAILS_IMAGE_TYPE_BITMAP && s_bitmap_image) {
    const GRect target =
        GRect(origin.x, origin.y, image_bounds.w, image_bounds.h);
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, s_bitmap_image, target);
    return;
  }

  if (s_content.image_type == DETAILS_IMAGE_TYPE_PDC && s_pdc_image) {
    gdraw_command_image_draw(ctx, s_pdc_image, origin);
  }
}

static void prv_update_image(void) {
  // Clean up old images
  if (s_pdc_image) {
    gdraw_command_image_destroy(s_pdc_image);
    s_pdc_image = NULL;
  }
  if (s_bitmap_image) {
    gbitmap_destroy(s_bitmap_image);
    s_bitmap_image = NULL;
  }

  // Load new image
  if (s_content.image_type == DETAILS_IMAGE_TYPE_BITMAP) {
    s_bitmap_image = gbitmap_create_with_resource(s_content.image_resource_id);
  } else {
    s_pdc_image = gdraw_command_image_create_with_resource(s_content.image_resource_id);
  }

  // Mark image layer for redraw
  if (s_image_layer) {
    layer_mark_dirty(s_image_layer);
  }
}

static void prv_format_additional_info(char *buffer, size_t buffer_size) {
  // Format altitude
  char alt_str[32];
  if (s_content.altitude_deg >= 0) {
    snprintf(alt_str, sizeof(alt_str), "%d° above horizon", s_content.altitude_deg);
  } else {
    snprintf(alt_str, sizeof(alt_str), "%d° below horizon", -s_content.altitude_deg);
  }

  // Format azimuth with cardinal direction
  char az_str[32];
  const char *directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
  int dir_index = ((s_content.azimuth_deg + 22) / 45) % 8;  // +22 for proper rounding
  snprintf(az_str, sizeof(az_str), "%d° %s", s_content.azimuth_deg, directions[dir_index]);

  // Format illumination (magnitude)
  char illum_str[32];
  int magnitude_int = s_content.illumination_x10 / 10;
  int magnitude_frac = abs(s_content.illumination_x10) % 10;
  if (s_content.illumination_x10 < 0) {
    snprintf(illum_str, sizeof(illum_str), "-%d.%d", -magnitude_int, magnitude_frac);
  } else {
    snprintf(illum_str, sizeof(illum_str), "+%d.%d", magnitude_int, magnitude_frac);
  }

  // Combine into formatted string
  snprintf(buffer, buffer_size,
           "Altitude\n%s\n\nAzimuth\n%s\n\nIllumination\n%s",
           alt_str, az_str, illum_str);
}

static void prv_update_content_display(void) {
  if (!s_window) {
    return;
  }

  // Update title
  if (s_title_layer) {
    text_layer_set_text(s_title_layer, s_content.title_text);
  }

  // Update detail text
  if (s_detail_layer) {
    text_layer_set_text(s_detail_layer, s_content.detail_text);
  }

  // Update grid text
  if (s_grid_layers[0][0]) text_layer_set_text(s_grid_layers[0][0], s_content.grid_top_left);
  if (s_grid_layers[0][1]) text_layer_set_text(s_grid_layers[0][1], s_content.grid_top_right);
  if (s_grid_layers[1][0]) text_layer_set_text(s_grid_layers[1][0], s_content.grid_bottom_left);
  if (s_grid_layers[1][1]) text_layer_set_text(s_grid_layers[1][1], s_content.grid_bottom_right);

  // Update long text with altitude, azimuth, and illumination info
  if (s_long_text_layer) {
    prv_format_additional_info(s_content.long_text, sizeof(s_content.long_text));

    text_layer_set_text(s_long_text_layer, s_content.long_text);
  }

  // Update image if needed
  prv_update_image();

  // Mark layers for redraw
  if (s_scroll_layer) {
    layer_mark_dirty(scroll_layer_get_layer(s_scroll_layer));
  }
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!prv_is_loading()) {
    options_menu_show();
  }
}

static void prv_scroll_up_handler(ClickRecognizerRef recognizer, void *context) {
  ScrollLayer *scroll_layer = (ScrollLayer *)context;
  if (!scroll_layer) {
    return;
  }

  const GPoint offset = scroll_layer_get_content_offset(scroll_layer);
  const bool within_one_screen = offset.y >= -s_page_height;
  scroll_layer_set_paging(scroll_layer, within_one_screen);
  scroll_layer_scroll_up_click_handler(recognizer, scroll_layer);
}

static void prv_scroll_down_handler(ClickRecognizerRef recognizer, void *context) {
  ScrollLayer *scroll_layer = (ScrollLayer *)context;
  if (!scroll_layer) {
    return;
  }

  const GPoint offset = scroll_layer_get_content_offset(scroll_layer);
  scroll_layer_set_paging(scroll_layer, offset.y == 0);
  scroll_layer_scroll_down_click_handler(recognizer, scroll_layer);
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_scroll_up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_scroll_down_handler);
  // Keep repeating up/down behavior consistent with defaults.
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, prv_scroll_up_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, prv_scroll_down_handler);
}

static void prv_create_grid_layers(GRect bounds, GFont font) {
  const int16_t column_width = (bounds.size.w - GRID_MARGIN * 3) / 2;
  const int16_t row_height = GRID_ROW_HEIGHT;
  int16_t y = bounds.origin.y;

  const char *grid_text[GRID_ROWS][GRID_COLS] = {
      {s_content.grid_top_left, s_content.grid_top_right},
      {s_content.grid_bottom_left, s_content.grid_bottom_right},
  };

  for (int row = 0; row < GRID_ROWS; ++row) {
    int16_t x = GRID_MARGIN;
    for (int col = 0; col < GRID_COLS; ++col) {
      GRect frame = GRect(x, y, column_width, row_height);
      s_grid_layers[row][col] = text_layer_create(frame);
      text_layer_set_text(s_grid_layers[row][col], grid_text[row][col]);
      text_layer_set_background_color(s_grid_layers[row][col], GColorClear);
      text_layer_set_text_color(s_grid_layers[row][col], layout_get()->foreground);
      text_layer_set_font(s_grid_layers[row][col], font);
      text_layer_set_overflow_mode(s_grid_layers[row][col], GTextOverflowModeWordWrap);
      text_layer_set_text_alignment(s_grid_layers[row][col],GTextAlignmentCenter);
      scroll_layer_add_child(s_scroll_layer,
                             text_layer_get_layer(s_grid_layers[row][col]));

      x += column_width + GRID_MARGIN;
    }
    y += row_height + GRID_MARGIN;
  }
}

static void prv_window_load(Window *window) {
  const Layout *layout = layout_get();
  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);
  s_page_height = bounds.size.h;

  s_status_layer = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_layer, layout->background, layout->foreground);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_layer));

  const GRect scroll_bounds = GRect(bounds.origin.x,
                                    bounds.origin.y + STATUS_BAR_LAYER_HEIGHT,
                                    bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT);
  s_scroll_layer = scroll_layer_create(scroll_bounds);
  scroll_layer_set_shadow_hidden(s_scroll_layer, true);
  scroll_layer_set_context(s_scroll_layer, s_scroll_layer);
  scroll_layer_set_callbacks(
      s_scroll_layer,
      (ScrollLayerCallbacks){
          .click_config_provider = prv_click_config_provider,
      });
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_set_paging(s_scroll_layer, true);

  // Title
  const int16_t side_margin = GRID_MARGIN;
  int16_t y_cursor = TITLE_TOP_MARGIN;
  const GFont title_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GRect title_frame =
      GRect(side_margin, y_cursor, bounds.size.w - side_margin * 2, 28);
  const GSize title_size =
      prv_calc_text_size(s_content.title_text, title_font, title_frame);
  title_frame.size.h = title_size.h > 0 ? title_size.h : 20;
  s_title_layer = text_layer_create(title_frame);
  text_layer_set_text(s_title_layer, s_content.title_text);
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_color(s_title_layer, layout->foreground);
  text_layer_set_font(s_title_layer, title_font);
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_title_layer));

  y_cursor += title_frame.size.h + TITLE_BOTTOM_MARGIN;

  // Hero image
  s_pdc_image = NULL;
  s_bitmap_image = NULL;
  if (s_content.image_type == DETAILS_IMAGE_TYPE_BITMAP) {
    s_bitmap_image = gbitmap_create_with_resource(s_content.image_resource_id);
  } else {
    s_pdc_image = gdraw_command_image_create_with_resource(s_content.image_resource_id);
  }
  const GSize hero_size = prv_get_image_size();
  const int16_t image_layer_height = hero_size.h + HERO_IMAGE_FRAME_PADDING;
  s_image_layer = layer_create(GRect(0, y_cursor, bounds.size.w, image_layer_height));
  layer_set_update_proc(s_image_layer, prv_draw_image);
  scroll_layer_add_child(s_scroll_layer, s_image_layer);
  y_cursor += image_layer_height + HERO_IMAGE_BOTTOM_MARGIN;

  // Detail text
  const GFont detail_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  GRect detail_frame = GRect(side_margin, y_cursor, bounds.size.w - side_margin * 2, 60);
  const GSize detail_size =
      prv_calc_text_size(s_content.detail_text, detail_font, detail_frame);
  detail_frame.size.h = detail_size.h > 0 ? detail_size.h : 20;
  s_detail_layer = text_layer_create(detail_frame);
  text_layer_set_text(s_detail_layer, s_content.detail_text);
  text_layer_set_background_color(s_detail_layer, GColorClear);
  text_layer_set_text_color(s_detail_layer, layout->foreground);
  text_layer_set_font(s_detail_layer, detail_font);
  text_layer_set_overflow_mode(s_detail_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_detail_layer, GTextAlignmentCenter);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_detail_layer));

  y_cursor += detail_frame.size.h + DETAIL_BOTTOM_MARGIN;

  // Grid values (2x2)
  const GRect grid_bounds =
      GRect(0, y_cursor, bounds.size.w, GRID_ROW_HEIGHT * GRID_ROWS + GRID_MARGIN);
  prv_create_grid_layers(grid_bounds, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  y_cursor += grid_bounds.size.h + LONG_TEXT_TOP_MARGIN;

  // Long-form text after the first "page"
  const GFont long_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  GRect long_frame = GRect(side_margin, y_cursor, bounds.size.w - side_margin * 2, bounds.size.h);
  const GSize long_size = prv_calc_text_size(s_content.long_text, long_font, long_frame);
  // long_frame.size.h = long_size.h;

  s_long_text_layer = text_layer_create(long_frame);
  text_layer_set_text(s_long_text_layer, s_content.long_text);
  text_layer_set_background_color(s_long_text_layer, GColorClear);
  text_layer_set_text_color(s_long_text_layer, layout->foreground);
  text_layer_set_font(s_long_text_layer, long_font);
  text_layer_set_overflow_mode(s_long_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_long_text_layer, GTextAlignmentLeft);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_long_text_layer));

  y_cursor += long_size.h + GRID_MARGIN;

  // Ensure at least two screens of scrollable content.
  const int16_t min_height = s_page_height * 2;
  const int16_t content_height = y_cursor > min_height ? y_cursor : min_height;
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, content_height));

  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void prv_window_unload(Window *window) {
  for (int row = 0; row < GRID_ROWS; ++row) {
    for (int col = 0; col < GRID_COLS; ++col) {
      if (s_grid_layers[row][col]) {
        text_layer_destroy(s_grid_layers[row][col]);
        s_grid_layers[row][col] = NULL;
      }
    }
  }

  if (s_long_text_layer) {
    text_layer_destroy(s_long_text_layer);
    s_long_text_layer = NULL;
  }
  if (s_detail_layer) {
    text_layer_destroy(s_detail_layer);
    s_detail_layer = NULL;
  }
  if (s_title_layer) {
    text_layer_destroy(s_title_layer);
    s_title_layer = NULL;
  }
  if (s_image_layer) {
    layer_destroy(s_image_layer);
    s_image_layer = NULL;
  }
  if (s_pdc_image) {
    gdraw_command_image_destroy(s_pdc_image);
    s_pdc_image = NULL;
  }
  if (s_bitmap_image) {
    gbitmap_destroy(s_bitmap_image);
    s_bitmap_image = NULL;
  }
  if (s_status_layer) {
    status_bar_layer_destroy(s_status_layer);
    s_status_layer = NULL;
  }
  if (s_scroll_layer) {
    scroll_layer_destroy(s_scroll_layer);
    s_scroll_layer = NULL;
  }
}

void details_init(void) {
  if (s_window) {
    return;
  }

  s_content = s_default_content;
  s_is_loading = false;
  s_window = window_create();
  window_set_background_color(s_window, layout_get()->background);
  window_set_window_handlers(s_window, (WindowHandlers){
                                        .load = prv_window_load,
                                        .unload = prv_window_unload,
                                    });
}

void details_deinit(void) {
  if (!s_window) {
    return;
  }

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
  s_scroll_layer = NULL;
  s_image_layer = NULL;
  s_title_layer = NULL;
  s_detail_layer = NULL;
  s_long_text_layer = NULL;
  s_pdc_image = NULL;
  s_bitmap_image = NULL;
  s_status_layer = NULL;
}

void details_show(const DetailsContent *content) {
  if (!s_window) {
    details_init();
  }

  // Update content
  s_content = s_default_content;
  if (content) {
    s_content = *content;
    s_is_loading = false;
  }

  // Check if window is already visible
  bool window_visible = window_stack_contains_window(s_window);

  if (window_visible) {
    // Window is already visible, update the content in place
    prv_update_content_display();
  } else {
    // Window not visible, push it to show
    window_stack_push(s_window, true);
  }
}

void details_show_body(int body_id) {
  if (!s_window) {
    details_init();
  }

  // Request body data from the phone
  if (bodymsg_request_body(body_id)) {
    // Show window with loading content
    s_content = s_loading_content;
    s_is_loading = true;
    window_stack_push(s_window, true);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to request body data for ID %d", body_id);
    // Fallback to default content if request fails
    s_content = s_default_content;
    window_stack_push(s_window, true);
  }
}

void details_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
  }
}

const DetailsContent* details_get_current_content(void) {
  return &s_content;
}