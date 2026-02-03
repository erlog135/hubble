#include "events.h"
#include "../style.h"
#include "../utils/bodymsg.h"
#include "../utils/logging.h"

static Window *s_window;
static TextLayer *s_text_layer;
static bool s_refresh_pending = false;

// Forward declarations
static void prv_inbox_received_callback(DictionaryIterator *iter, void *context);
static void prv_inbox_dropped_callback(AppMessageResult reason, void *context);
static void prv_outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context);

static void prv_request_events_refresh(void) {
  if (!bodymsg_is_ready()) {
    HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "AppMessage not ready for events refresh");
    text_layer_set_text(s_text_layer, "\n\n\nConnection Error");
    return;
  }

  // Prepare the outbox buffer
  DictionaryIterator *out_iter;
  AppMessageResult result = app_message_outbox_begin(&out_iter);

  if (result != APP_MSG_OK) {
    HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Error preparing outbox: %d", (int)result);
    text_layer_set_text(s_text_layer, "\n\n\nSend Error");
    return;
  }

  // Add request flag
  int value = 1;
  dict_write_int(out_iter, MESSAGE_KEY_REQUEST_EVENTS_REFRESH, &value, sizeof(int), true);

  // Send the message
  result = app_message_outbox_send();
  if (result != APP_MSG_OK) {
    HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Error sending request: %d", (int)result);
    text_layer_set_text(s_text_layer, "\n\n\nSend Error");
    return;
  }

  s_refresh_pending = true;
  text_layer_set_text(s_text_layer, "\n\n\nRefreshing...");
  HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Requested events refresh");
}

static void prv_window_load(Window *window) {
  const Layout *layout = layout_get();

  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  // Create text layer
  s_text_layer = text_layer_create(bounds);
  text_layer_set_background_color(s_text_layer, layout->background);
  text_layer_set_text_color(s_text_layer, layout->foreground);
  text_layer_set_text(s_text_layer, "\n\n\nEvents");
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_appear(Window *window) {
  // Request events refresh when window appears
  prv_request_events_refresh();
}

static void prv_inbox_received_callback(DictionaryIterator *iter, void *context) {
  // Check if this is an EVENTS_REFRESHED message
  Tuple *events_refreshed_tuple = dict_find(iter, MESSAGE_KEY_EVENTS_REFRESHED);
  if (events_refreshed_tuple && s_refresh_pending) {
    int32_t event_count = events_refreshed_tuple->value->int32;
    s_refresh_pending = false;

    if (event_count >= 0) {
      // Success - show count
      static char buffer[64];
      snprintf(buffer, sizeof(buffer), "\n\n\nRefreshed %d events", (int)event_count);
      text_layer_set_text(s_text_layer, buffer);
      HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Events refresh completed: %d events", (int)event_count);
    } else {
      // Error
      text_layer_set_text(s_text_layer, "\n\n\nRefresh Failed");
      HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Events refresh failed");
    }
  }
}

static void prv_inbox_dropped_callback(AppMessageResult reason, void *context) {
  HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Events message dropped. Reason: %d", (int)reason);
  if (s_refresh_pending) {
    s_refresh_pending = false;
    text_layer_set_text(s_text_layer, "\n\n\nMessage Error");
  }
}

static void prv_outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Events message send failed. Reason: %d", (int)reason);
  if (s_refresh_pending) {
    s_refresh_pending = false;
    text_layer_set_text(s_text_layer, "\n\n\nSend Failed");
  }
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  s_text_layer = NULL;
}

void events_init(void) {
  if (s_window) {
    return;
  }

  // Initialize bodymsg if not already initialized (for message routing)
  if (!bodymsg_is_ready()) {
    bodymsg_init();
    bodymsg_register_callbacks();
  }

  // Deregister body message callbacks to allow events to handle messages
  bodymsg_deregister_callbacks();

  // Register events message callbacks
  app_message_register_inbox_received(prv_inbox_received_callback);
  app_message_register_inbox_dropped(prv_inbox_dropped_callback);
  app_message_register_outbox_failed(prv_outbox_failed_callback);

  s_window = window_create();
  window_set_background_color(s_window, layout_get()->background);
  window_set_window_handlers(s_window, (WindowHandlers){
                                    .load = prv_window_load,
                                    .appear = prv_window_appear,
                                    .unload = prv_window_unload,
                                });
}

void events_deinit(void) {
  if (!s_window) {
    return;
  }

  // Unregister events message callbacks
  app_message_register_inbox_received(NULL);
  app_message_register_inbox_dropped(NULL);
  app_message_register_outbox_failed(NULL);

  // Re-register body message callbacks
  bodymsg_register_callbacks();

  window_stack_remove(s_window, false);
  window_destroy(s_window);
  s_window = NULL;
  s_text_layer = NULL;
  s_refresh_pending = false;
}

void events_show(void) {
  if (!s_window) {
    events_init();
  }
  window_stack_push(s_window, true);
}

void events_hide(void) {
  if (s_window) {
    window_stack_remove(s_window, true);
  }
}