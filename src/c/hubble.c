#include <pebble.h>
#include "windows/home.h"
#include "utils/bodymsg.h"

static void prv_init(void) {
  bodymsg_init();
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
