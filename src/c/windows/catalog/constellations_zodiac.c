#include "constellations_zodiac.h"
#include "../body/details.h"
#include "../../style.h"
#include "../../utils/bodymsg.h"
#include "../../utils/logging.h"

#define ZODIAC_COUNT 12

// Body IDs for each menu item (must match order in menu)
static const int ZODIAC_BODY_IDS[ZODIAC_COUNT] = {
  10,  // Aries
  11,  // Taurus
  12,  // Gemini
  13,  // Cancer
  14,  // Leo
  15,  // Virgo
  16,  // Libra
  17,  // Scorpius
  18,  // Sagittarius
  19,  // Capricornus
  20,  // Aquarius
  21   // Pisces
};

static Window *s_window;
static SimpleMenuLayer *s_menu_layer;
static SimpleMenuSection s_menu_sections[1];
static SimpleMenuItem s_menu_items[ZODIAC_COUNT];

static void prv_menu_select_callback(int index, void *context) {
  HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Zodiac constellation menu selected: %s (body ID: %d)",
          s_menu_items[index].title, ZODIAC_BODY_IDS[index]);

  if (index >= 0 && index < ZODIAC_COUNT) {
    int body_id = ZODIAC_BODY_IDS[index];
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

  s_menu_items[0] = (SimpleMenuItem){
      .title = "Aries",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[1] = (SimpleMenuItem){
      .title = "Taurus",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[2] = (SimpleMenuItem){
      .title = "Gemini",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[3] = (SimpleMenuItem){
      .title = "Cancer",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[4] = (SimpleMenuItem){
      .title = "Leo",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[5] = (SimpleMenuItem){
      .title = "Virgo",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[6] = (SimpleMenuItem){
      .title = "Libra",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[7] = (SimpleMenuItem){
      .title = "Scorpius",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[8] = (SimpleMenuItem){
      .title = "Sagittarius",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[9] = (SimpleMenuItem){
      .title = "Capricornus",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[10] = (SimpleMenuItem){
      .title = "Aquarius",
      .callback = prv_menu_select_callback,
  };
  s_menu_items[11] = (SimpleMenuItem){
      .title = "Pisces",
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

void constellations_zodiac_menu_init(void) {
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

void constellations_zodiac_menu_deinit(void) {
  if (!s_window) {
    return;
  }

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
  s_menu_layer = NULL;
}

void constellations_zodiac_menu_show(void) {
  if (!s_window) {
    constellations_zodiac_menu_init();
  }
  window_stack_push(s_window, true);
}

void constellations_zodiac_menu_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
  }
}
