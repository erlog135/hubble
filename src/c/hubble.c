#include <pebble.h>
#include "providers/altitude_provider.h"
#include "windows/targeter.h"

static void prv_on_altitude(int16_t altitude_deg) {
  targeter_set_current_altitude(altitude_deg);
}

static void prv_init(void) {
  targeter_init();

  altitude_provider_init();
  altitude_provider_set_handler(prv_on_altitude);

  targeter_show();
}

static void prv_deinit(void) {
  altitude_provider_deinit();
  targeter_hide();
  targeter_deinit();
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
