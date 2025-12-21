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
  "Pluto"
};

// Resource IDs for body images (using RESOURCE_ID_FULL_MOON as placeholder)
// TODO: Replace with actual resource IDs when images are available
const uint32_t BODY_RESOURCE_IDS[] = {
  RESOURCE_ID_FULL_MOON,  // Moon
  RESOURCE_ID_PLANET_MERCURY,  // Mercury
  RESOURCE_ID_PLANET_VENUS,  // Venus
  RESOURCE_ID_PLANET_MARS,  // Mars
  RESOURCE_ID_PLANET_JUPITER,  // Jupiter
  RESOURCE_ID_PLANET_SATURN,  // Saturn
  RESOURCE_ID_PLANET_URANUS,  // Uranus
  RESOURCE_ID_PLANET_NEPTUNE,  // Neptune
  RESOURCE_ID_PLANET_PLUTO   // Pluto
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
