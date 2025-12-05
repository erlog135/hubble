#include "azimuth_provider.h"

static int16_t s_azimuth_deg = 0;
static AzimuthUpdateHandler s_handler = NULL;

static void prv_handle_heading(CompassHeadingData data) {
  if (data.compass_status == CompassStatusDataInvalid) {
    return;
  }

  // Pebble heading is counter-clockwise from 12 o'clock; convert to CW degrees.
  const int32_t cw_trig_angle = TRIG_MAX_ANGLE - data.magnetic_heading;
  int32_t deg = TRIGANGLE_TO_DEG(cw_trig_angle);

  // Normalize to 0-359.
  if (deg >= 360) {
    deg -= 360;
  } else if (deg < 0) {
    deg = 0;
  }

  s_azimuth_deg = (int16_t)deg;

  if (s_handler) {
    s_handler(s_azimuth_deg);
  }
}

void azimuth_provider_init(void) {
  compass_service_subscribe(prv_handle_heading);
  // Default 1-degree filter is fine; can be adjusted by callers later if needed.
}

void azimuth_provider_deinit(void) {
  compass_service_unsubscribe();
  s_handler = NULL;
}

void azimuth_provider_set_handler(AzimuthUpdateHandler handler) {
  s_handler = handler;
  if (handler) {
    handler(s_azimuth_deg);
  }
}

int16_t azimuth_provider_get_azimuth_deg(void) {
  return s_azimuth_deg;
}

