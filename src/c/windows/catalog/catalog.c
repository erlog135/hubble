#include "catalog.h"
#include "../../style.h"

static Window *s_window;
static SimpleMenuLayer *s_menu_layer;
static SimpleMenuSection s_menu_sections[1];
static SimpleMenuItem s_menu_items[4];
static StatusBarLayer *s_status_layer;
static GBitmap *s_menu_icon;

static void prv_menu_select_callback(int index, void *context) {
  vibes_short_pulse();
  APP_LOG(APP_LOG_LEVEL_INFO, "Catalog menu selected: %s", s_menu_items[index].title);
}

static void prv_window_load(Window *window) {
  const Layout *layout = layout_get();

  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  s_status_layer = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_layer, layout->background, layout->foreground);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_layer));

  if (!s_menu_icon) {
    s_menu_icon = gbitmap_create_with_resource(RESOURCE_ID_MENU_ICON);
  }

  s_menu_items[0] = (SimpleMenuItem){
      .title = "Moons",
      .icon = s_menu_icon,
      .callback = prv_menu_select_callback,
  };
  s_menu_items[1] = (SimpleMenuItem){
      .title = "Planets",
      .icon = s_menu_icon,
      .callback = prv_menu_select_callback,
  };
  s_menu_items[2] = (SimpleMenuItem){
      .title = "Stars",
      .icon = s_menu_icon,
      .callback = prv_menu_select_callback,
  };
  s_menu_items[3] = (SimpleMenuItem){
      .title = "Constellations",
      .icon = s_menu_icon,
      .callback = prv_menu_select_callback,
  };

  s_menu_sections[0] = (SimpleMenuSection){
      .num_items = ARRAY_LENGTH(s_menu_items),
      .items = s_menu_items,
  };

  const GRect menu_frame = GRect(bounds.origin.x, bounds.origin.y + STATUS_BAR_LAYER_HEIGHT,
                                 bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT);
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

  status_bar_layer_destroy(s_status_layer);
  s_status_layer = NULL;

  if (s_menu_icon) {
    gbitmap_destroy(s_menu_icon);
    s_menu_icon = NULL;
  }
}

void catalog_menu_init(void) {
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

void catalog_menu_deinit(void) {
  if (!s_window) {
    return;
  }

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
  s_menu_layer = NULL;
  s_status_layer = NULL;

  if (s_menu_icon) {
    gbitmap_destroy(s_menu_icon);
    s_menu_icon = NULL;
  }
}

void catalog_menu_show(void) {
  if (!s_window) {
    catalog_menu_init();
  }
  window_stack_push(s_window, true);
}

void catalog_menu_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
  }
}
