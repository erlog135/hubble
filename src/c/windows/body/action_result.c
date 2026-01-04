#include "action_result.h"
#include "../../style.h"

static Window *s_window;
static TextLayer *s_text_layer;

static void prv_window_load(Window *window) {
  const Layout *layout = layout_get();
  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  const GRect text_frame = GRect(bounds.origin.x + 20, bounds.origin.y + 20,
                                 bounds.size.w - 40, bounds.size.h - 40);
  s_text_layer = text_layer_create(text_frame);
  text_layer_set_text_color(s_text_layer, layout->foreground);
  text_layer_set_background_color(s_text_layer, layout->background);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
  if (s_text_layer) {
    text_layer_destroy(s_text_layer);
    s_text_layer = NULL;
  }
}

void action_result_init(void) {
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

void action_result_deinit(void) {
  if (!s_window) {
    return;
  }

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
}

void action_result_show(const char *message) {
  if (!s_window) {
    action_result_init();
  }

  if (s_text_layer) {
    text_layer_set_text(s_text_layer, message);
  }

  window_stack_push(s_window, true);
}

void action_result_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
  }
}
