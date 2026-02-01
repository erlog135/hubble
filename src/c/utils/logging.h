#pragma once

#include <pebble.h>

// Define HUBBLE_LOGGING_ENABLED to enable logging
// Undefine or set to 0 to disable logging
#define HUBBLE_LOGGING_ENABLED 0

#if HUBBLE_LOGGING_ENABLED
    // When enabled, HUBBLE_LOG is identical to APP_LOG
    #define HUBBLE_LOG(level, fmt, args...) APP_LOG(level, fmt, ##args)
#else
    // When disabled, HUBBLE_LOG does nothing
    #define HUBBLE_LOG(level, fmt, args...) ((void)0)
#endif
