#include "favorites.h"
#include "../style.h"
#include "../utils/settings.h"
#include "../utils/body_info.h"
#include "../utils/logging.h"
#include "./body/details.h"

static Window *s_window;
static SimpleMenuLayer *s_menu_layer;
static TextLayer *s_text_layer;
static SimpleMenuSection s_menu_sections[1];
static SimpleMenuItem *s_menu_items = NULL;
static int s_num_favorites = 0;

static void prv_count_favorites() {
  s_num_favorites = 0;
  for (int i = 0; i < NUM_BODIES && i < 32; i++) {
    if (settings_get()->favorites & (1 << i)) {
      s_num_favorites++;
    }
  }
}

static void prv_menu_select_callback(int index, void *context) {
  // Find the actual body ID from the favorites list
  int favorite_index = 0;
  for (int body_id = 0; body_id < NUM_BODIES && body_id < 32; body_id++) {
    if (settings_get()->favorites & (1 << body_id)) {
      if (favorite_index == index) {
        details_show_body(body_id);
        return;
      }
      favorite_index++;
    }
  }
}

static void prv_window_load(Window *window) {
  const Layout *layout = layout_get();
  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  prv_count_favorites();

  HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Settings: %d", settings_get()->favorites);
  HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Favorites: %d", s_num_favorites);

  if (s_num_favorites > 0) {
    // Create menu with favorites
    s_menu_items = malloc(sizeof(SimpleMenuItem) * s_num_favorites);
    if (!s_menu_items) {
      HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Failed to allocate memory for favorites menu items");
      return;
    }

    int item_index = 0;
    for (int body_id = 0; body_id < NUM_BODIES && body_id < 32; body_id++) {
      if (settings_get()->favorites & (1 << body_id)) {
        s_menu_items[item_index] = (SimpleMenuItem){
          .title = body_info_get_name(body_id),
          .callback = prv_menu_select_callback,
        };
        item_index++;
      }
    }

    s_menu_sections[0] = (SimpleMenuSection){
      .title = PBL_IF_ROUND_ELSE("        Favorites", "Favorites"),
      .num_items = s_num_favorites,
      .items = s_menu_items,
    };

    const GRect menu_frame = GRect(bounds.origin.x, bounds.origin.y,
                                   bounds.size.w, bounds.size.h);
    s_menu_layer = simple_menu_layer_create(menu_frame, window, s_menu_sections,
                                           ARRAY_LENGTH(s_menu_sections), NULL);
    MenuLayer *menu_layer = simple_menu_layer_get_menu_layer(s_menu_layer);
    menu_layer_set_normal_colors(menu_layer, layout->background, layout->foreground);
    menu_layer_set_highlight_colors(menu_layer, layout->highlight, layout->highlight_foreground);
    layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
  } else {
    // Show "No favorites yet" text
    const GRect text_frame = GRect(bounds.origin.x + 10, bounds.origin.y + 10,
                                   bounds.size.w - 20, bounds.size.h - 20);
    s_text_layer = text_layer_create(text_frame);
    text_layer_set_text(s_text_layer, "No favorites yet");
    text_layer_set_text_color(s_text_layer, layout->foreground);
    text_layer_set_background_color(s_text_layer, layout->background);
    text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
    text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  }
}

static void prv_window_unload(Window *window) {
  if (s_menu_layer) {
    simple_menu_layer_destroy(s_menu_layer);
    s_menu_layer = NULL;
  }

  if (s_text_layer) {
    text_layer_destroy(s_text_layer);
    s_text_layer = NULL;
  }

  if (s_menu_items) {
    free(s_menu_items);
    s_menu_items = NULL;
  }

  s_num_favorites = 0;
}

void favorites_init(void) {
  if (s_window) {
    return;
  }

  s_window = window_create();
  window_set_background_color(s_window, layout_get()->background);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
}

void favorites_deinit(void) {
  if (!s_window) {
    return;
  }

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
}

void favorites_show(void) {
  if (!s_window) {
    favorites_init();
  }
  window_stack_push(s_window, true);
}

void favorites_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
  }
}

Window* favorites_get_window(void) {
  return s_window;
}