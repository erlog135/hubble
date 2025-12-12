#pragma once

#include <pebble.h>

// Body names corresponding to BODY_NAMES in JavaScript msgproc.js
extern const char* BODY_NAMES[];
extern const int NUM_BODIES;

// Resource IDs for body images (using RESOURCE_ID_FULL_MOON as placeholder)
extern const uint32_t BODY_RESOURCE_IDS[];

// Get body name by ID, returns NULL if invalid ID
const char* body_info_get_name(int body_id);

// Get body resource ID by ID, returns RESOURCE_ID_FULL_MOON if invalid ID
uint32_t body_info_get_resource_id(int body_id);
