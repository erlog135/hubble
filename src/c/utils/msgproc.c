#include "msgproc.h"
#include "body_info.h"
#include <string.h>

// BodyPackage bit field layout constants
#define BODY_ID_BITS 8
#define AZIMUTH_BITS 9
#define ALTITUDE_BITS 8
#define RISE_HOUR_BITS 5
#define RISE_MINUTE_BITS 6
#define SET_HOUR_BITS 5
#define SET_MINUTE_BITS 6
#define LUMINANCE_BITS 9
#define PHASE_BITS 3
#define PADDING_BITS 5

// Sentinel values for invalid times
#define SENTINEL_HOUR 31
#define SENTINEL_MIN 63

// Phase names for Moon
static const char* MOON_PHASES[] = {
    "New Moon",
    "Waxing Crescent",
    "First Quarter",
    "Waxing Gibbous",
    "Full Moon",
    "Waning Gibbous",
    "Third Quarter",
    "Waning Crescent"
};

// Static buffers for formatted strings
static char s_time_buffer[16];
static char s_angle_buffer[16];

// Helper function to read bits from the byte array
static uint32_t read_bits(const uint8_t *data, size_t data_len, int *bit_pos, int num_bits) {
    uint32_t result = 0;

    for (int i = 0; i < num_bits; i++) {
        if (*bit_pos >= (int)(data_len * 8)) {
            // Out of bounds
            return 0;
        }

        int byte_index = *bit_pos / 8;
        int bit_index = *bit_pos % 8;

        if (data[byte_index] & (1 << bit_index)) {
            result |= (1 << i);
        }

        (*bit_pos)++;
    }

    return result;
}

// Decode signed value from bit field
static int32_t decode_signed(uint32_t value, int bits) {
    uint32_t sign_bit = 1 << (bits - 1);
    if (value & sign_bit) {
        // Negative value
        return (int32_t)(value - (1 << bits));
    }
    return (int32_t)value;
}

bool msgproc_unpack_body_package(const uint8_t *data, size_t length, DetailsContent *content) {
    if (!data || length != 8 || !content) {
        return false;
    }

    int bit_pos = 0;

    // Read body ID (8 bits)
    uint32_t body_id = read_bits(data, length, &bit_pos, BODY_ID_BITS);
    if ((int)body_id >= NUM_BODIES) {
        return false;
    }

    // Read azimuth (9 bits, unsigned 0-360)
    uint32_t azimuth = read_bits(data, length, &bit_pos, AZIMUTH_BITS);

    // Read altitude (8 bits, signed -90 to 90)
    uint32_t alt_raw = read_bits(data, length, &bit_pos, ALTITUDE_BITS);
    int32_t altitude = decode_signed(alt_raw, ALTITUDE_BITS);

    // Read rise time (5 bits hour + 6 bits minute)
    uint32_t rise_hour = read_bits(data, length, &bit_pos, RISE_HOUR_BITS);
    uint32_t rise_minute = read_bits(data, length, &bit_pos, RISE_MINUTE_BITS);

    // Read set time (5 bits hour + 6 bits minute)
    uint32_t set_hour = read_bits(data, length, &bit_pos, SET_HOUR_BITS);
    uint32_t set_minute = read_bits(data, length, &bit_pos, SET_MINUTE_BITS);

    // Read luminance (9 bits signed, represents value * 10)
    uint32_t lum_raw = read_bits(data, length, &bit_pos, LUMINANCE_BITS);
    int32_t luminance_x10 = decode_signed(lum_raw, LUMINANCE_BITS);

    // Read phase (3 bits, 0-7, only used for Moon)
    uint32_t phase = read_bits(data, length, &bit_pos, PHASE_BITS);

    // Skip padding (5 bits)
    read_bits(data, length, &bit_pos, PADDING_BITS);

    // Get body name and resource ID
    const char *body_name = body_info_get_name(body_id);
    if (!body_name) {
        return false;
    }

    uint32_t resource_id = body_info_get_resource_id(body_id);

    // Determine if this is a moon (Moon has body_id 0)
    bool is_moon = (body_id == 0);

    // Format the content structure
    content->title_text = body_name;

    // Detail text: for Moon show phase, for others show basic info
    if (is_moon && phase < sizeof(MOON_PHASES)/sizeof(MOON_PHASES[0])) {
        content->detail_text = MOON_PHASES[phase];
    } else {
        // For planets, show altitude and azimuth
        static char detail_buf[32];
        if (altitude >= 0) {
            snprintf(detail_buf, sizeof(detail_buf), "%d° above horizon", (int)altitude);
        } else {
            snprintf(detail_buf, sizeof(detail_buf), "%d° below horizon", -(int)altitude);
        }
        content->detail_text = detail_buf;
    }

    // Grid: Rise/Set times
    content->grid_top_left = "RISE";
    content->grid_top_right = "SET";

    if (rise_hour != SENTINEL_HOUR && rise_minute != SENTINEL_MIN) {
        content->grid_bottom_left = msgproc_format_time(rise_hour, rise_minute);
    } else {
        content->grid_bottom_left = "--:--";
    }

    if (set_hour != SENTINEL_HOUR && set_minute != SENTINEL_MIN) {
        content->grid_bottom_right = msgproc_format_time(set_hour, set_minute);
    } else {
        content->grid_bottom_right = "--:--";
    }

    // Long text: detailed information
    static char long_text_buf[512];

    if (is_moon) {
        // Moon-specific description
        const char *phase_name = (phase < sizeof(MOON_PHASES)/sizeof(MOON_PHASES[0])) ?
                                 MOON_PHASES[phase] : "Unknown Phase";
        // Format luminance as integer with decimal (since Pebble doesn't support floats)
        int luminance_int = luminance_x10 / 10;
        int luminance_frac = abs(luminance_x10) % 10;
        char luminance_str[16];
        if (luminance_x10 < 0) {
            snprintf(luminance_str, sizeof(luminance_str), "-%d.%d", -luminance_int, luminance_frac);
        } else {
            snprintf(luminance_str, sizeof(luminance_str), "%d.%d", luminance_int, luminance_frac);
        }

        snprintf(long_text_buf, sizeof(long_text_buf),
            "Earth's Moon is the fifth largest natural satellite in the Solar "
            "System and the only place beyond Earth where humans have set foot. "
            "Currently in %s phase. Azimuth: %d°, Altitude: %d°, "
            "Luminance: %s.",
            phase_name, (int)azimuth, (int)altitude, luminance_str);
    } else {
        // Planet description
        const char *rise_time = (rise_hour != SENTINEL_HOUR) ?
                                msgproc_format_time(rise_hour, rise_minute) : "N/A";
        const char *set_time = (set_hour != SENTINEL_HOUR) ?
                               msgproc_format_time(set_hour, set_minute) : "N/A";

        // Format magnitude as integer with decimal (since Pebble doesn't support floats)
        int magnitude_int = luminance_x10 / 10;
        int magnitude_frac = abs(luminance_x10) % 10;
        char magnitude_str[16];
        if (luminance_x10 < 0) {
            snprintf(magnitude_str, sizeof(magnitude_str), "-%d.%d", -magnitude_int, magnitude_frac);
        } else {
            snprintf(magnitude_str, sizeof(magnitude_str), "%d.%d", magnitude_int, magnitude_frac);
        }

        snprintf(long_text_buf, sizeof(long_text_buf),
            "%s is one of the planets visible from Earth. "
            "Current position: Azimuth %d°, Altitude %d°. "
            "Rises at %s, sets at %s. "
            "Apparent magnitude: %s.",
            body_name, (int)azimuth, (int)altitude, rise_time, set_time, magnitude_str);
    }

    content->long_text = long_text_buf;
    content->image_resource_id = resource_id;

    // Determine image type (Saturn uses PDC, others use bitmap)
    content->image_type = (body_id == 5) ? DETAILS_IMAGE_TYPE_PDC : DETAILS_IMAGE_TYPE_BITMAP;

    return true;
}

const char* msgproc_format_time(int hour, int minute) {
    if (hour == SENTINEL_HOUR || minute == SENTINEL_MIN) {
        return "--:--";
    }

    // Convert to 12-hour format
    bool is_pm = (hour >= 12);
    int display_hour = hour % 12;
    if (display_hour == 0) display_hour = 12;

    snprintf(s_time_buffer, sizeof(s_time_buffer), "%d:%02d%s",
             display_hour, minute, is_pm ? "PM" : "AM");

    return s_time_buffer;
}

const char* msgproc_format_angle(int degrees, bool is_azimuth) {
    if (is_azimuth) {
        // Azimuth: show as degrees with cardinal direction
        const char *directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
        int dir_index = ((degrees + 22) / 45) % 8;  // +22 for proper rounding
        snprintf(s_angle_buffer, sizeof(s_angle_buffer), "%d°%s", degrees, directions[dir_index]);
    } else {
        // Altitude: just show degrees with sign
        snprintf(s_angle_buffer, sizeof(s_angle_buffer), "%d°", degrees);
    }

    return s_angle_buffer;
}
