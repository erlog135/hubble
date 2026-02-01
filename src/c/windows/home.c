#include "home.h"
#include "favorites.h"
#include "events.h"
#include "./catalog/planets.h"
#include "./catalog/constellations_zodiac.h"
#include "./catalog/constellations.h"
#include "./body/details.h"
#include "../style.h"
#include "../utils/logging.h"

static Window *s_window;
static SimpleMenuLayer *s_menu_layer;
static SimpleMenuSection s_menu_sections[2];
static SimpleMenuItem s_main_items[2];
static SimpleMenuItem s_catalog_items[5]; 

static void prv_main_menu_select_callback(int index, void *context) {

  switch (index) {
    case 0:  // Favorites
      favorites_show();
      break;
    case 1:  // Refresh Events
      events_show();
      break;
    default:
      HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Home menu selected: %s", s_main_items[index].title);
      vibes_short_pulse();
      break;
  }
}

static void prv_catalog_menu_select_callback(int index, void *context) {

  HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Catalog menu selected: %s", s_catalog_items[index].title);

  switch (index) {
    case 0:  // Moon
      details_show_body(0);
      break;
    case 1:  // Planets
      planets_menu_show();
      break;
    case 2:  // The Sun
      details_show_body(9);
      break;
    case 3:  // Constellations - Zodiac
      constellations_zodiac_menu_show();
      break;
    case 4:  // Constellations - Other
      constellations_menu_show();
      break;
    default:
      vibes_short_pulse();
      break;
  }
}

static void prv_window_load(Window *window) {
  const Layout *layout = layout_get();

  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  // Main menu items
  s_main_items[0] = (SimpleMenuItem){
      .title = "Favorites",
      .callback = prv_main_menu_select_callback,
  };
  s_main_items[1] = (SimpleMenuItem){
      .title = "Refresh Events",
      .callback = prv_main_menu_select_callback,
  };

  // Catalog items
  s_catalog_items[0] = (SimpleMenuItem){
      .title = "The Moon",
      .callback = prv_catalog_menu_select_callback,
  };
  s_catalog_items[1] = (SimpleMenuItem){
      .title = "Planets  >",
      .callback = prv_catalog_menu_select_callback,
  };
  s_catalog_items[2] = (SimpleMenuItem){
      .title = "The Sun",
      .callback = prv_catalog_menu_select_callback,
  };
  s_catalog_items[3] = (SimpleMenuItem){
      .title = "Constellations  >",
      .subtitle = "Zodiac",
      .callback = prv_catalog_menu_select_callback,
  };
  s_catalog_items[4] = (SimpleMenuItem){
      .title = "Constellations  >",
      .subtitle = "Other",
      .callback = prv_catalog_menu_select_callback,
  };

  // Main menu section
  s_menu_sections[0] = (SimpleMenuSection){
      .title = PBL_IF_ROUND_ELSE("        Menu", "Menu"),
      .num_items = ARRAY_LENGTH(s_main_items),
      .items = s_main_items,
  };

  // Catalog section
  s_menu_sections[1] = (SimpleMenuSection){
      .title = PBL_IF_ROUND_ELSE("        Catalog", "Catalog"),
      .num_items = ARRAY_LENGTH(s_catalog_items),
      .items = s_catalog_items,
  };

  const GRect menu_frame = GRect(bounds.origin.x, bounds.origin.y,
                                 bounds.size.w, bounds.size.h);
  s_menu_layer = simple_menu_layer_create(menu_frame, window, s_menu_sections,
                                          ARRAY_LENGTH(s_menu_sections), NULL);
  MenuLayer *menu_layer = simple_menu_layer_get_menu_layer(s_menu_layer);
  menu_layer_set_normal_colors(menu_layer, layout->background, layout->foreground);
  menu_layer_set_highlight_colors(menu_layer, layout->highlight, layout->highlight_foreground);
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
}

static void prv_window_unload(Window *window) {
  simple_menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
}

void home_init(void) {
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

void home_deinit(void) {
  if (!s_window) {
    return;
  }

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
  s_menu_layer = NULL;
}

void home_show(void) {
  if (!s_window) {
    home_init();
  }
  window_stack_push(s_window, true);
}

void home_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
  }
}
