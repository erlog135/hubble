#include "body_info.h"

// Body names corresponding to BODY_NAMES in JavaScript msgproc.js
// Must stay in sync with the JavaScript array
const char* BODY_NAMES[] = {
  "Moon",
  "Mercury",
  "Venus",
  "Mars",
  "Jupiter",
  "Saturn",
  "Uranus",
  "Neptune",
  "Pluto",
  "Sun",
  "Aries",
  "Taurus",
  "Gemini",
  "Cancer",
  "Leo",
  "Virgo",
  "Libra",
  "Scorpius",
  "Sagittarius",
  "Capricornus",
  "Aquarius",
  "Pisces",
  "Orion",
  "Ursa Major",
  "Ursa Minor",
  "Cassiopeia",
  "Cygnus",
  "Crux",
  "Lyra"
};

// Resource IDs for body images (using RESOURCE_ID_FULL_MOON as placeholder)
// TODO: Replace with actual resource IDs when images are available
const uint32_t BODY_RESOURCE_IDS[] = {
  RESOURCE_ID_FULL_MOON,
  RESOURCE_ID_PLANET_MERCURY,
  RESOURCE_ID_PLANET_VENUS,
  RESOURCE_ID_PLANET_MARS,
  RESOURCE_ID_PLANET_JUPITER,
  RESOURCE_ID_PLANET_SATURN,
  RESOURCE_ID_PLANET_URANUS,
  RESOURCE_ID_PLANET_NEPTUNE,
  RESOURCE_ID_PLANET_PLUTO,
  RESOURCE_ID_SUN,
  RESOURCE_ID_CONSTELLATION_ARIES,
  RESOURCE_ID_CONSTELLATION_TAURUS,
  RESOURCE_ID_CONSTELLATION_GEMINI,
  RESOURCE_ID_CONSTELLATION_CANCER,
  RESOURCE_ID_CONSTELLATION_LEO,
  RESOURCE_ID_CONSTELLATION_VIRGO,
  RESOURCE_ID_CONSTELLATION_LIBRA,
  RESOURCE_ID_CONSTELLATION_SCORPIUS,
  RESOURCE_ID_CONSTELLATION_SAGITTARIUS,
  RESOURCE_ID_CONSTELLATION_CAPRICORNUS,
  RESOURCE_ID_CONSTELLATION_AQUARIUS,
  RESOURCE_ID_CONSTELLATION_PISCES,
  RESOURCE_ID_CONSTELLATION_ORION,
  RESOURCE_ID_CONSTELLATION_URSA_MAJOR,
  RESOURCE_ID_CONSTELLATION_URSA_MINOR,
  RESOURCE_ID_CONSTELLATION_CASSIOPEIA,
  RESOURCE_ID_CONSTELLATION_CYGNUS,
  RESOURCE_ID_CONSTELLATION_CRUX,
  RESOURCE_ID_CONSTELLATION_LYRA
};

const int NUM_BODIES = sizeof(BODY_NAMES) / sizeof(BODY_NAMES[0]);

const char* body_info_get_name(int body_id) {
  if (body_id >= 0 && body_id < NUM_BODIES) {
    return BODY_NAMES[body_id];
  }
  return NULL;
}

uint32_t body_info_get_resource_id(int body_id) {
  if (body_id >= 0 && body_id < NUM_BODIES) {
    return BODY_RESOURCE_IDS[body_id];
  }
  return RESOURCE_ID_FULL_MOON;  // Default fallback
}
