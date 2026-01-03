#include "home.h"
#include "favorites.h"
#include "catalog/catalog.h"
#include "../style.h"

static Window *s_window;
static SimpleMenuLayer *s_menu_layer;
static SimpleMenuSection s_menu_sections[1];
static SimpleMenuItem s_menu_items[4];

static void prv_menu_select_callback(int index, void *context) {

  switch (index) {
    case 0:  // Favorites
      favorites_show();
      break;
    case 2:  // Catalog
      catalog_menu_show();
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_INFO, "Home menu selected: %s", s_menu_items[index].title);
      vibes_short_pulse();
      break;
  }
}

static void prv_window_load(Window *window) {
  const Layout *layout = layout_get();

  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  s_menu_items[0] = (SimpleMenuItem){
      .title = "Favorites",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[1] = (SimpleMenuItem){
      .title = "Upcoming",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[2] = (SimpleMenuItem){
      .title = "Catalog",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[3] = (SimpleMenuItem){
      .title = "Settings",
      .callback = prv_menu_select_callback,
  };

  s_menu_sections[0] = (SimpleMenuSection){
      .num_items = ARRAY_LENGTH(s_menu_items),
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
