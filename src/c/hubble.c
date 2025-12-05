#include <pebble.h>
#include "windows/targeter.h"

static void prv_init(void) {
  targeter_init();
  targeter_show();
}

static void prv_deinit(void) {
  targeter_hide();
  targeter_deinit();
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
