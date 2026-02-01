#include <pebble.h>
#include "windows/home.h"
#include "utils/settings.h"
#include "utils/logging.h"

static void prv_init(void) {
  settings_load();
  HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Settings: %d", settings.favorites);
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
