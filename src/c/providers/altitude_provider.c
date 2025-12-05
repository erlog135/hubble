#include "altitude_provider.h"
#include <math.h>
#include <stdlib.h>

// The accelerometer reports milli-Gs. 1000 mg ~= 1g.
#define MG_PER_G 1000

static int16_t s_altitude_deg = 0;
static AltitudeUpdateHandler s_handler = NULL;

static int32_t prv_int_sqrt(int32_t v) {
  if (v <= 0) {
    return 0;
  }
  // Simple integer sqrt using floating sqrt for clarity; v is small (< 32M).
  return (int32_t)sqrt((double)v);
}

static int16_t prv_calc_altitude_deg(const AccelData *sample) {
  const int32_t y = sample->y;  // milli-G
  const int32_t x = sample->x;
  const int32_t z = sample->z;

  // Use y magnitude against horizontal magnitude to infer tilt angle.
  const int32_t horiz = prv_int_sqrt((x * x) + (z * z));
  const int32_t safe_horiz = horiz == 0 ? 1 : horiz;

  // atan2_lookup returns a TRIG angle; convert to degrees.
  const int32_t trig_angle = atan2_lookup(abs(y), safe_horiz);
  int32_t deg = TRIGANGLE_TO_DEG(trig_angle);

  // We only care about 0-90° (horizon to straight up).
  if (deg > 90) {
    deg = 90;
  } else if (deg < 0) {
    deg = 0;
  }

  return (int16_t)deg;
}

static void prv_accel_handler(AccelData *data, uint32_t num_samples) {
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
  accel_data_service_subscribe(1, prv_accel_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
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

