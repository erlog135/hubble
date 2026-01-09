#include <pebble.h>
#include "windows/home.h"
#include "utils/bodymsg.h"
#include "utils/settings.h"

static void prv_init(void) {
  settings_load();
  APP_LOG(APP_LOG_LEVEL_INFO, "Settings: %d", settings.favorites);
  bodymsg_init();
  bodymsg_register_callbacks();  // Register body message callbacks by default
  home_init();
  home_show();
}

static void prv_deinit(void) {
  home_hide();
  home_deinit();
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
