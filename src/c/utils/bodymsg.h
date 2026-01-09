#pragma once

#include <pebble.h>

// Initialize the body message system
// Sets up AppMessage buffers and registers callbacks
void bodymsg_init(void);

// Deinitialize the body message system
void bodymsg_deinit(void);

// Request data for a specific body ID
// This sends a REQUEST_BODY message to the JavaScript side
bool bodymsg_request_body(int body_id);

// Check if the message system is ready to send messages
bool bodymsg_is_ready(void);

// Register body message callbacks (for when handling body requests)
void bodymsg_register_callbacks(void);

// Deregister body message callbacks (to allow other handlers like events)
void bodymsg_deregister_callbacks(void);
