#include "details.h"
#include "../../style.h"
#include "../../utils/bodymsg.h"
#include "../../utils/logging.h"
#include "options.h"
#include "action_indicator.h"

#define HERO_IMAGE_SIZE 50
#define CONSTELLATION_IMAGE_SIZE 80
#define CONSTELLATION_BODY_ID_START 10
#define GRID_MARGIN 0
#define GRID_ROUND_SIDE_PADDING 8
#define GRID_ROWS 2
#define GRID_COLS 2

#ifdef PBL_PLATFORM_EMERY
  #define FONT_HEIGHT 28
  #define GRID_ROW_HEIGHT 24
#else
  #define FONT_HEIGHT 21
  #define GRID_ROW_HEIGHT PBL_IF_ROUND_ELSE(18, 14)
#endif

#define TITLE_TOP_MARGIN 0
#define TITLE_BOTTOM_MARGIN 4
#define HERO_IMAGE_FRAME_PADDING 6
#define HERO_IMAGE_BOTTOM_MARGIN 0
#define DETAIL_BOTTOM_MARGIN 0
#define LONG_TEXT_TOP_MARGIN 4

#ifdef PBL_PLATFORM_EMERY
  char *title_font_key = FONT_KEY_GOTHIC_24_BOLD;
  char *grid_font_key = FONT_KEY_GOTHIC_24_BOLD;
  char *detail_font_key = FONT_KEY_GOTHIC_24;
#else
  char *title_font_key = FONT_KEY_GOTHIC_18_BOLD;
  char *grid_font_key = PBL_IF_ROUND_ELSE(FONT_KEY_GOTHIC_18, FONT_KEY_GOTHIC_14_BOLD);
  char *detail_font_key = FONT_KEY_GOTHIC_18;
#endif

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
static Layer *s_action_indicator_layer;
static Layer *s_content_indicator_layer;
static ContentIndicator *s_content_indicator;

static int16_t s_page_height;
static DetailsContent s_content;
static bool s_is_loading;

static bool prv_is_loading(void) {
  return s_is_loading;
}

static const DetailsContent s_loading_content = {
    .title_text = "Loading...",
    .detail_text = "Fetching data",
    .grid_top_left = "RISE",
    .grid_top_right = "SET",
    .grid_bottom_left = "--:--",
    .grid_bottom_right = "--:--",
    .long_text = "Loading...\nLoading...",
    .image_resource_id = 0,  // No image during loading
    .image_type = DETAILS_IMAGE_TYPE_BITMAP,
    .azimuth_deg = 0,
    .altitude_deg = 0,
    .illumination_x10 = 0,
    .body_id = -1,  // Not a specific body
};

static bool prv_is_constellation(void) {
  return s_content.body_id >= CONSTELLATION_BODY_ID_START;
}

static GSize prv_get_image_size(void) {
  if (prv_is_constellation()) {
    return GSize(CONSTELLATION_IMAGE_SIZE, CONSTELLATION_IMAGE_SIZE);
  }
  return GSize(HERO_IMAGE_SIZE, HERO_IMAGE_SIZE);
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

  // Load new image only if resource_id is valid (non-zero)
  if (s_content.image_resource_id != 0) {
    if (s_content.image_type == DETAILS_IMAGE_TYPE_BITMAP) {
      s_bitmap_image = gbitmap_create_with_resource(s_content.image_resource_id);
    } else {
      s_pdc_image = gdraw_command_image_create_with_resource(s_content.image_resource_id);
    }
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

  // Combine into formatted string.
  // If the body is the Sun or beyond (>= 9), do not display illumination.
  if (s_content.body_id >= 9) {
    snprintf(buffer, buffer_size,
             "Altitude\n%s\n\nAzimuth\n%s",
             alt_str, az_str);
  } else {
    // Format illumination (magnitude)
    char illum_str[32];
    int magnitude_int = s_content.illumination_x10 / 10;
    int magnitude_frac = abs(s_content.illumination_x10) % 10;
    if (s_content.illumination_x10 < 0) {
      snprintf(illum_str, sizeof(illum_str), "-%d.%d", -magnitude_int, magnitude_frac);
    } else {
      snprintf(illum_str, sizeof(illum_str), "+%d.%d", magnitude_int, magnitude_frac);
    }

    snprintf(buffer, buffer_size,
             "Altitude\n%s\n\nAzimuth\n%s\n\nIllumination\n%s",
             alt_str, az_str, illum_str);
  }
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

  // Update grid text - empty for constellations
  if (prv_is_constellation()) {
    if (s_grid_layers[0][0]) text_layer_set_text(s_grid_layers[0][0], "");
    if (s_grid_layers[0][1]) text_layer_set_text(s_grid_layers[0][1], "");
    if (s_grid_layers[1][0]) text_layer_set_text(s_grid_layers[1][0], "");
    if (s_grid_layers[1][1]) text_layer_set_text(s_grid_layers[1][1], "");
  } else {
    if (s_grid_layers[0][0]) text_layer_set_text(s_grid_layers[0][0], s_content.grid_top_left);
    if (s_grid_layers[0][1]) text_layer_set_text(s_grid_layers[0][1], s_content.grid_top_right);
    if (s_grid_layers[1][0]) text_layer_set_text(s_grid_layers[1][0], s_content.grid_bottom_left);
    if (s_grid_layers[1][1]) text_layer_set_text(s_grid_layers[1][1], s_content.grid_bottom_right);
  }

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

static void prv_draw_content_indicator_background(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void prv_content_offset_changed_handler(ScrollLayer *scroll_layer, void *context) {
  if (!s_content_indicator) {
    return;
  }
  
  const GPoint offset = scroll_layer_get_content_offset(scroll_layer);
  // Only show indicator when scrolled to the very top (offset.y == 0)
  //except no it doesn't it shows it all the time until it's at the very bottom
  const bool at_top = (offset.y == 0);
  content_indicator_set_content_available(s_content_indicator, ContentIndicatorDirectionDown, at_top);
  //so i conditionally hide it again, for real
  layer_set_hidden(s_content_indicator_layer, !at_top);
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

  // Empty grid text for constellations
  const char *grid_text[GRID_ROWS][GRID_COLS];
  if (prv_is_constellation()) {
    grid_text[0][0] = "";
    grid_text[0][1] = "";
    grid_text[1][0] = "";
    grid_text[1][1] = "";
  } else {
    grid_text[0][0] = s_content.grid_top_left;
    grid_text[0][1] = s_content.grid_top_right;
    grid_text[1][0] = s_content.grid_bottom_left;
    grid_text[1][1] = s_content.grid_bottom_right;
  }

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

  // Create action indicator (initially hidden during loading)
  s_action_indicator_layer = action_indicator_create(bounds);
  action_indicator_add_to_window(window);
  action_indicator_set_visible(!prv_is_loading());

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
          .content_offset_changed_handler = prv_content_offset_changed_handler,
      });
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_set_paging(s_scroll_layer, true);

  // Set up content indicator
  s_content_indicator = scroll_layer_get_content_indicator(s_scroll_layer);
  
  // Create a layer for the indicator background (black background)
  const int16_t indicator_height = 20;
  const GRect indicator_frame = GRect(0, scroll_bounds.size.h - indicator_height, 
                                      scroll_bounds.size.w, indicator_height);
  s_content_indicator_layer = layer_create(indicator_frame);
  layer_set_update_proc(s_content_indicator_layer, prv_draw_content_indicator_background);
  layer_add_child(scroll_layer_get_layer(s_scroll_layer), s_content_indicator_layer);
  
  // Configure the down direction with white arrow
  ContentIndicatorConfig indicator_config = {
    .layer = s_content_indicator_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = GColorWhite,
      .background = GColorBlack,
    }
  };
  content_indicator_configure_direction(s_content_indicator, ContentIndicatorDirectionDown, &indicator_config);
  
  // Initially hidden (will be shown when at top via callback)
  content_indicator_set_content_available(s_content_indicator, ContentIndicatorDirectionDown, false);

  // Title
  const int16_t side_margin = GRID_MARGIN;
  int16_t y_cursor = TITLE_TOP_MARGIN;
  GRect title_frame =
      GRect(side_margin, y_cursor, bounds.size.w - side_margin * 2, FONT_HEIGHT);
  s_title_layer = text_layer_create(title_frame);
  text_layer_set_text(s_title_layer, s_content.title_text);
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_color(s_title_layer, layout->foreground);
  text_layer_set_font(s_title_layer, fonts_get_system_font(title_font_key));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_title_layer));

  y_cursor += title_frame.size.h + TITLE_BOTTOM_MARGIN;

  // Hero image
  s_pdc_image = NULL;
  s_bitmap_image = NULL;
  // Only load image if resource_id is valid (non-zero)
  if (s_content.image_resource_id != 0) {
    if (s_content.image_type == DETAILS_IMAGE_TYPE_BITMAP) {
      s_bitmap_image = gbitmap_create_with_resource(s_content.image_resource_id);
    } else {
      s_pdc_image = gdraw_command_image_create_with_resource(s_content.image_resource_id);
    }
  }
  // Use actual image size for current content
  const GSize hero_size = prv_get_image_size();
  const int16_t image_layer_height = hero_size.h + HERO_IMAGE_FRAME_PADDING;

  if (PBL_IF_ROUND_ELSE(1, 0)) {
    // Round watch: grid columns on left and right of image, image centered in window (horizontally and vertically)
    // Use raw image dimensions (no padding) and full window dimensions (no status bar) for perfect centering
    const int16_t grid_row_height = GRID_ROW_HEIGHT;
    
    // Center the image horizontally in the full window using raw image dimensions
    const int16_t image_center_x = bounds.size.w / 2;
    const int16_t image_start_x = image_center_x - (hero_size.w / 2);
    
    // Calculate available space on each side of the centered image
    const int16_t left_space = image_start_x - GRID_ROUND_SIDE_PADDING;
    const int16_t right_space = bounds.size.w - (image_start_x + hero_size.w) - GRID_ROUND_SIDE_PADDING;
    const int16_t grid_column_width = (left_space < right_space ? left_space : right_space) - GRID_MARGIN;
    
    // Position grid columns on either side of the centered image
    const int16_t left_grid_x = GRID_ROUND_SIDE_PADDING;
    const int16_t right_grid_x = image_start_x + hero_size.w + GRID_MARGIN;
    
    // Center the image vertically in the full window using raw image dimensions
    const int16_t image_center_y = bounds.size.h / 2;
    const int16_t image_start_y = image_center_y - (hero_size.h / 2) - STATUS_BAR_LAYER_HEIGHT;
    const int16_t grid_y = image_start_y + (hero_size.h - (grid_row_height * GRID_ROWS + GRID_MARGIN)) / 2;
    
    // Image layer dimensions (include padding for drawing)
    const int16_t image_layer_width = hero_size.w + HERO_IMAGE_FRAME_PADDING;
    const int16_t image_layer_height_round = hero_size.h + HERO_IMAGE_FRAME_PADDING;
    // Position layer so that when drawing function centers image within layer, image is centered in window
    // Drawing function centers at (layer.w/2, layer.h/2) relative to layer origin
    // We want image center at (bounds.size.w/2, bounds.size.h/2)
    const int16_t image_layer_x = image_center_x - (image_layer_width / 2);
    const int16_t image_layer_y = image_center_y - (image_layer_height_round / 2) - STATUS_BAR_LAYER_HEIGHT;

    // Left column (RISE) - empty for constellations
    for (int row = 0; row < GRID_ROWS; ++row) {
      GRect frame = GRect(left_grid_x, grid_y + row * (grid_row_height + GRID_MARGIN), grid_column_width, grid_row_height);
      s_grid_layers[row][0] = text_layer_create(frame);
      const char *text = prv_is_constellation() ? "" : (row == 0 ? s_content.grid_top_left : s_content.grid_bottom_left);
      text_layer_set_text(s_grid_layers[row][0], text);
      text_layer_set_background_color(s_grid_layers[row][0], GColorClear);
      text_layer_set_text_color(s_grid_layers[row][0], layout->foreground);
      text_layer_set_font(s_grid_layers[row][0], fonts_get_system_font(grid_font_key));
      text_layer_set_overflow_mode(s_grid_layers[row][0], GTextOverflowModeWordWrap);
      text_layer_set_text_alignment(s_grid_layers[row][0], GTextAlignmentCenter);
      scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_grid_layers[row][0]));
    }

    // Image centered in window (horizontally and vertically)
    // Layer is positioned to account for padding, but image itself is perfectly centered
    s_image_layer = layer_create(GRect(image_layer_x, image_layer_y, image_layer_width, image_layer_height_round));
    layer_set_update_proc(s_image_layer, prv_draw_image);
    scroll_layer_add_child(s_scroll_layer, s_image_layer);

    // Right column (SET) - empty for constellations
    for (int row = 0; row < GRID_ROWS; ++row) {
      GRect frame = GRect(right_grid_x, grid_y + row * (grid_row_height + GRID_MARGIN), grid_column_width, grid_row_height);
      s_grid_layers[row][1] = text_layer_create(frame);
      const char *text = prv_is_constellation() ? "" : (row == 0 ? s_content.grid_top_right : s_content.grid_bottom_right);
      text_layer_set_text(s_grid_layers[row][1], text);
      text_layer_set_background_color(s_grid_layers[row][1], GColorClear);
      text_layer_set_text_color(s_grid_layers[row][1], layout->foreground);
      text_layer_set_font(s_grid_layers[row][1], fonts_get_system_font(grid_font_key));
      text_layer_set_overflow_mode(s_grid_layers[row][1], GTextOverflowModeWordWrap);
      text_layer_set_text_alignment(s_grid_layers[row][1], GTextAlignmentCenter);
      scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_grid_layers[row][1]));
    }

    y_cursor = image_layer_y + image_layer_height_round + HERO_IMAGE_BOTTOM_MARGIN;
  } else {
    // Non-round watch: current layout (image, detail text, then grid)
    s_image_layer = layer_create(GRect(0, y_cursor, bounds.size.w, image_layer_height));
    layer_set_update_proc(s_image_layer, prv_draw_image);
    scroll_layer_add_child(s_scroll_layer, s_image_layer);
    y_cursor += image_layer_height + HERO_IMAGE_BOTTOM_MARGIN;

    // Detail text
    GRect detail_frame = GRect(side_margin, y_cursor, bounds.size.w - side_margin * 2, FONT_HEIGHT);
    s_detail_layer = text_layer_create(detail_frame);
    text_layer_set_text(s_detail_layer, s_content.detail_text);
    text_layer_set_background_color(s_detail_layer, GColorClear);
    text_layer_set_text_color(s_detail_layer, layout->foreground);
    text_layer_set_font(s_detail_layer, fonts_get_system_font(detail_font_key));
    text_layer_set_overflow_mode(s_detail_layer, GTextOverflowModeWordWrap);
    text_layer_set_text_alignment(s_detail_layer, GTextAlignmentCenter);
    scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_detail_layer));

    y_cursor += detail_frame.size.h + DETAIL_BOTTOM_MARGIN;

    // Grid values (2x2)
    const GRect grid_bounds =
        GRect(0, y_cursor, bounds.size.w, GRID_ROW_HEIGHT * GRID_ROWS + GRID_MARGIN);
    prv_create_grid_layers(grid_bounds, fonts_get_system_font(grid_font_key));
    y_cursor += grid_bounds.size.h;
  }

  // Detail text (for round watches, placed after image)
  if (PBL_IF_ROUND_ELSE(1, 0)) {
    GRect detail_frame = GRect(side_margin, y_cursor, bounds.size.w - side_margin * 2, 21);
    s_detail_layer = text_layer_create(detail_frame);
    text_layer_set_text(s_detail_layer, s_content.detail_text);
    text_layer_set_background_color(s_detail_layer, GColorClear);
    text_layer_set_text_color(s_detail_layer, layout->foreground);
    text_layer_set_font(s_detail_layer, fonts_get_system_font(detail_font_key));
    text_layer_set_overflow_mode(s_detail_layer, GTextOverflowModeWordWrap);
    text_layer_set_text_alignment(s_detail_layer, GTextAlignmentCenter);
    scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_detail_layer));

    y_cursor += detail_frame.size.h + DETAIL_BOTTOM_MARGIN;
  }

  // Adjust spacing so first page ends at exactly scroll_bounds.size.h
  // The scrollable area height is bounds.size.h - STATUS_BAR_LAYER_HEIGHT
  const int16_t scroll_height = bounds.size.h - STATUS_BAR_LAYER_HEIGHT;
  const int16_t min_spacing = LONG_TEXT_TOP_MARGIN;
  const int16_t target_y = scroll_height;
  
  if (y_cursor + min_spacing <= target_y) {
    // Can fit minimum spacing and reach target height
    y_cursor = target_y;
  } else {
    // Content too tall, use minimum spacing
    y_cursor += min_spacing;
  }

  // Long-form text after the first "page"
  GRect long_frame = GRect(side_margin, y_cursor, bounds.size.w - side_margin * 2, bounds.size.h);

  s_long_text_layer = text_layer_create(long_frame);
  text_layer_set_text(s_long_text_layer, s_content.long_text);
  text_layer_set_background_color(s_long_text_layer, GColorClear);
  text_layer_set_text_color(s_long_text_layer, layout->foreground);
  text_layer_set_font(s_long_text_layer, fonts_get_system_font(detail_font_key));
  text_layer_set_overflow_mode(s_long_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_long_text_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_long_text_layer));

  y_cursor += bounds.size.h + GRID_MARGIN;

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
  if (s_content_indicator_layer) {
    layer_destroy(s_content_indicator_layer);
    s_content_indicator_layer = NULL;
  }
  s_content_indicator = NULL;  // This is owned by scroll_layer, don't destroy
  action_indicator_destroy();
  if (s_scroll_layer) {
    scroll_layer_destroy(s_scroll_layer);
    s_scroll_layer = NULL;
  }
}

void details_init(void) {
  if (s_window) {
    return;
  }

  // Initialize bodymsg system for body data requests and declination handling
  bodymsg_init();
  bodymsg_register_callbacks();
  
  s_content = s_loading_content;
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
  s_content_indicator_layer = NULL;
  s_content_indicator = NULL;
  
  // Deinitialize bodymsg system
  bodymsg_deregister_callbacks();
  bodymsg_deinit();
}

void details_show(const DetailsContent *content) {
  if (!s_window) {
    details_init();
  }

  // Check if body type is changing (regular body <-> constellation)
  bool old_is_constellation = prv_is_constellation();
  
  // Update content
  if (content) {
    s_content = *content;
    s_is_loading = false;

    // Show action indicator now that loading is complete
    action_indicator_set_visible(true);

    // Deregister bodymsg callbacks after successfully receiving body data
    // This allows other windows (like events) to take control of message handling
    bodymsg_deregister_callbacks();
    HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Details received body data, deregistered bodymsg callbacks");
  }

  bool new_is_constellation = prv_is_constellation();
  
  // Check if window is already visible
  bool window_visible = window_stack_contains_window(s_window);

  if (window_visible && old_is_constellation != new_is_constellation) {
    // Body type changed, need to recreate window with correct layout
    //^ does that even happen?
    window_stack_remove(s_window, false);
    prv_window_unload(s_window);
    prv_window_load(s_window);
    window_stack_push(s_window, false);
  } else if (window_visible) {
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

  // Ensure callbacks are registered before requesting body data
  // (They may have been deregistered after a previous request)
  if (bodymsg_is_ready()) {
    bodymsg_register_callbacks();
  }

  // Request body data from the phone
  if (bodymsg_request_body(body_id)) {
    // Show window with loading content
    s_content = s_loading_content;
    // Set the body_id so layout is calculated correctly for constellations
    s_content.body_id = body_id;
    s_is_loading = true;
    // Hide action indicator during loading
    action_indicator_set_visible(false);
    window_stack_push(s_window, true);
  } else {
    HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Failed to request body data for ID %d", body_id);
    // Don't show window if request fails
    // User can try again or return to previous screen
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