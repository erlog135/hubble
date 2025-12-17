#pragma once

#include <pebble.h>
#include "../windows/body/details.h"

// Unpack a BodyPackage (8-byte array) into a DetailsContent structure
// Returns true on success, false on failure
bool msgproc_unpack_body_package(const uint8_t *data, size_t length, DetailsContent *content);

// Helper function to format time strings for rise/set display
// Formats into the provided buffer and returns a pointer to it
const char* msgproc_format_time(int hour, int minute, char *buffer, size_t buffer_size);

// Helper function to format azimuth/altitude for display
const char* msgproc_format_angle(int degrees, bool is_azimuth);
