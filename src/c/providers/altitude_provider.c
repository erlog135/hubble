#include "altitude_provider.h"
static int16_t s_altitude_deg = 0;
static AltitudeUpdateHandler s_handler = NULL;

static int32_t prv_trig_to_signed_deg(int32_t trig_angle) {
  int32_t deg = TRIGANGLE_TO_DEG(trig_angle);
  return (deg > 180) ? deg - 360 : deg;
}

static int16_t prv_calc_altitude_deg(const AccelRawData *sample) {
  // Ignore X; assume watch is held parallel to line of sight.
  // Altitude is measured from looking straight ahead (0°) up to +90° (up) and
  // down to -90° (down) using Z (up) and Y (forward/back).
  const int32_t y = sample->y;  // milli-G, forward is negative
  const int32_t z = sample->z;  // milli-G, up is positive

  const int32_t trig_angle = atan2_lookup(z, -y);  // (-y) makes forward = 0°
  int32_t deg = prv_trig_to_signed_deg(trig_angle);

  if (deg > 90) {
    deg = 90;
  } else if (deg < -90) {
    deg = -90;
  }

  return (int16_t)deg;
}

static void prv_accel_handler(AccelRawData *data, uint32_t num_samples,
                              uint64_t timestamp) {
  if (num_samples == 0) {
    return;
  }

  const int16_t altitude = prv_calc_altitude_deg(&data[0]);
  s_altitude_deg = altitude;

  if (s_handler) {
    s_handler(altitude);
  }
}

void altitude_provider_init(void) {
  accel_raw_data_service_subscribe(1, prv_accel_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
  s_altitude_deg = 0;
}

void altitude_provider_deinit(void) {
  accel_data_service_unsubscribe();
  s_handler = NULL;
}

void altitude_provider_set_handler(AltitudeUpdateHandler handler) {
  s_handler = handler;
  if (handler) {
    handler(s_altitude_deg);
  }
}

int16_t altitude_provider_get_altitude_deg(void) {
  return s_altitude_deg;
}

