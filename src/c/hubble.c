#include <pebble.h>
#include "windows/home.h"

static void prv_init(void) {
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
