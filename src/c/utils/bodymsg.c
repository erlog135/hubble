#include "bodymsg.h"
#include "msgproc.h"
#include "../windows/body/details.h"
#include "logging.h"
#include <pebble.h>

// Message buffer sizes
#define INBOX_SIZE 64
#define OUTBOX_SIZE 64

// Static variables
static bool s_app_message_ready = false;
static int s_pending_body_id = -1;  // Body ID we're waiting for

// Forward declarations for callbacks
static void prv_inbox_received_callback(DictionaryIterator *iter, void *context);
static void prv_inbox_dropped_callback(AppMessageResult reason, void *context);
static void prv_outbox_sent_callback(DictionaryIterator *iter, void *context);
static void prv_outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context);

void bodymsg_init(void) {
    // Open AppMessage with appropriate buffer sizes
    app_message_open(INBOX_SIZE, OUTBOX_SIZE);

    // Note: Callbacks are registered separately via bodymsg_register_callbacks()
    // to allow dynamic switching between body messages and events

    s_app_message_ready = true;
    s_pending_body_id = -1;
}

void bodymsg_deinit(void) {
    s_app_message_ready = false;
    s_pending_body_id = -1;
}

bool bodymsg_is_ready(void) {
    return s_app_message_ready;
}

void bodymsg_register_callbacks(void) {
    if (!s_app_message_ready) {
        HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Cannot register callbacks: AppMessage not ready");
        return;
    }

    // Register callbacks for body message handling
    app_message_register_inbox_received(prv_inbox_received_callback);
    app_message_register_inbox_dropped(prv_inbox_dropped_callback);
    app_message_register_outbox_sent(prv_outbox_sent_callback);
    app_message_register_outbox_failed(prv_outbox_failed_callback);

    HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Body message callbacks registered");
}

void bodymsg_deregister_callbacks(void) {
    // Deregister callbacks to allow other handlers (like events) to take over
    app_message_register_inbox_received(NULL);
    app_message_register_inbox_dropped(NULL);
    app_message_register_outbox_sent(NULL);
    app_message_register_outbox_failed(NULL);

    HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Body message callbacks deregistered");
}

bool bodymsg_request_body(int body_id) {
    if (!s_app_message_ready) {
        HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "AppMessage not ready");
        return false;
    }

    if (s_pending_body_id != -1 && s_pending_body_id != body_id) {
        HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Cancelling pending request for body %d, requesting body %d instead",
                s_pending_body_id, body_id);
    }

    // Prepare the outbox buffer
    DictionaryIterator *out_iter;
    AppMessageResult result = app_message_outbox_begin(&out_iter);

    if (result != APP_MSG_OK) {
        HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Error preparing outbox: %d", (int)result);
        return false;
    }

    // Add the body ID to request
    dict_write_int(out_iter, MESSAGE_KEY_REQUEST_BODY, &body_id, sizeof(int), true);

    // Send the message
    result = app_message_outbox_send();
    if (result != APP_MSG_OK) {
        HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Error sending request: %d", (int)result);
        return false;
    }

    s_pending_body_id = body_id;
    HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Requested data for body %d", body_id);
    return true;
}

// Callback when a message is received
static void prv_inbox_received_callback(DictionaryIterator *iter, void *context) {
    HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Message received");

    // Check if this is a BODY_PACKAGE message
    Tuple *body_package_tuple = dict_find(iter, MESSAGE_KEY_BODY_PACKAGE);
    if (body_package_tuple) {
        // Get the data array
        if (body_package_tuple->type == TUPLE_BYTE_ARRAY) {
            uint8_t *data = body_package_tuple->value->data;
            uint16_t length = body_package_tuple->length;

            if (length == 7) {
                // Unpack the body package
                DetailsContent content;
                if (msgproc_unpack_body_package(data, length, &content)) {
                    // Only process if this response matches our current pending request
                    // We need to check if there's even a pending request
                    if (s_pending_body_id == -1) {
                        HUBBLE_LOG(APP_LOG_LEVEL_WARNING, "Received body data but no request was pending");
                        return;
                    }

                    // Show the details window
                    details_show(&content);
                    s_pending_body_id = -1;  // Clear pending request
                    HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Successfully unpacked and displayed body data");
                } else {
                    HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Failed to unpack body package");
                }
            } else {
                HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Invalid body package length: %d", length);
            }
        } else {
            HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Body package is not a byte array");
        }
    } else {
        HUBBLE_LOG(APP_LOG_LEVEL_WARNING, "Received message without BODY_PACKAGE key");
    }
}

// Callback when a message was received but dropped
static void prv_inbox_dropped_callback(AppMessageResult reason, void *context) {
    HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
    s_pending_body_id = -1;  // Clear any pending request
}

// Callback when a message was sent successfully
static void prv_outbox_sent_callback(DictionaryIterator *iter, void *context) {
    HUBBLE_LOG(APP_LOG_LEVEL_INFO, "Message sent successfully");
    // Note: We don't clear s_pending_body_id here because we're waiting for the response
}

// Callback when a message failed to send
static void prv_outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
    HUBBLE_LOG(APP_LOG_LEVEL_ERROR, "Message send failed. Reason: %d", (int)reason);
    s_pending_body_id = -1;  // Clear pending request on failure
}
