#include "constellations.h"
#include "../body/details.h"
#include "../../style.h"
#include "../../utils/bodymsg.h"
#include "../../utils/logging.h"
#include "../../utils/body_info.h"

#define CONSTELLATION_START_ID 22
#define CONSTELLATION_END_ID 28
#define CONSTELLATION_COUNT (CONSTELLATION_END_ID - CONSTELLATION_START_ID + 1)

static Window *s_window;
static SimpleMenuLayer *s_menu_layer;
static SimpleMenuSection s_menu_sections[1];
static SimpleMenuItem s_menu_items[CONSTELLATION_COUNT];

static void prv_menu_select_callback(int index, void *context) {
  int body_id = CONSTELLATION_START_ID + index;
  HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Constellation menu selected: %s (body ID: %d)",
          s_menu_items[index].title, body_id);

  if (index >= 0 && index < CONSTELLATION_COUNT) {
    details_show_body(body_id);
  } else {
    HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Invalid menu index: %d", index);
    details_show(NULL);
  }
}

static void prv_window_load(Window *window) {
  const Layout *layout = layout_get();

  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  // Build menu items from body ID range
  for (int i = 0; i < CONSTELLATION_COUNT; i++) {
    int body_id = CONSTELLATION_START_ID + i;
    s_menu_items[i] = (SimpleMenuItem){
        .title = body_info_get_name(body_id),
        .callback = prv_menu_select_callback,
    };
  }

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

void constellations_menu_init(void) {
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

void constellations_menu_deinit(void) {
  if (!s_window) {
    return;
  }

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
  s_menu_layer = NULL;
}

void constellations_menu_show(void) {
  if (!s_window) {
    constellations_menu_init();
  }
  window_stack_push(s_window, true);
}

void constellations_menu_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
    window_destroy(s_window);
    s_window = NULL;
  }
}
