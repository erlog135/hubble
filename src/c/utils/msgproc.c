#include "msgproc.h"
#include "body_info.h"
#include <string.h>

// BodyPackage bit field layout constants
#define BODY_ID_BITS 5
#define AZIMUTH_BITS 9
#define ALTITUDE_BITS 8
#define RISE_HOUR_BITS 5
#define RISE_MINUTE_BITS 6
#define SET_HOUR_BITS 5
#define SET_MINUTE_BITS 6
#define LUMINANCE_BITS 9
#define PHASE_BITS 3

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

// Resource IDs for Moon phases
static const uint32_t MOON_PHASE_RESOURCE_IDS[] = {
    RESOURCE_ID_NEW_MOON,
    RESOURCE_ID_WAXING_CRESCENT,
    RESOURCE_ID_FIRST_QUARTER,
    RESOURCE_ID_WAXING_GIBBOUS,
    RESOURCE_ID_FULL_MOON,
    RESOURCE_ID_WANING_GIBBOUS,
    RESOURCE_ID_THIRD_QUARTER,
    RESOURCE_ID_WANING_CRESCENT
};

// Static buffers for formatted strings
static char s_rise_time_buffer[16];
static char s_set_time_buffer[16];
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
    if (!data || length != 7 || !content) {
        return false;
    }

    int bit_pos = 0;

    // Read body ID (5 bits)
    uint32_t body_id = read_bits(data, length, &bit_pos, BODY_ID_BITS);
    if ((int)body_id >= NUM_BODIES) {
        return false;
    }

    // Read phase (3 bits, 0-7, only used for Moon)
    uint32_t phase = read_bits(data, length, &bit_pos, PHASE_BITS);

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

    // Get body name and resource ID
    const char *body_name = body_info_get_name(body_id);
    if (!body_name) {
        return false;
    }
    
    // Determine body characteristics
    bool is_moon = (body_id == 0);
    bool can_have_rise_set = (body_id <= 9);  // Moon (0), planets (1-8), and Sun (9)

    uint32_t resource_id;
    if (is_moon && phase < sizeof(MOON_PHASE_RESOURCE_IDS)/sizeof(MOON_PHASE_RESOURCE_IDS[0])) {
        // For Moon, use phase-specific resource ID
        resource_id = MOON_PHASE_RESOURCE_IDS[phase];
    } else {
        // For planets and other bodies, use the default resource ID
        resource_id = body_info_get_resource_id(body_id);
    }


    // Format the content structure
    snprintf(content->title_text, sizeof(content->title_text), "%s", body_name);
    content->body_id = body_id;

    // Detail text: for Moon show phase, for others show basic info
    if (is_moon && phase < sizeof(MOON_PHASES)/sizeof(MOON_PHASES[0])) {
        snprintf(content->detail_text, sizeof(content->detail_text), "%s", MOON_PHASES[phase]);
    } else {
        // For planets, show altitude and azimuth
        if (altitude >= 0) {
            snprintf(content->detail_text, sizeof(content->detail_text), "%d° above horizon", (int)altitude);
        } else {
            snprintf(content->detail_text, sizeof(content->detail_text), "%d° below horizon", -(int)altitude);
        }
    }

    // Grid: Rise/Set times for bodies that can have them calculated, Azimuth/Altitude for others

    if (can_have_rise_set) {
        // For Moon and planets: show rise/set times
        snprintf(content->grid_top_left, sizeof(content->grid_top_left), "RISE");
        snprintf(content->grid_top_right, sizeof(content->grid_top_right), "SET");

        if (rise_hour != SENTINEL_HOUR && rise_minute != SENTINEL_MIN) {
            msgproc_format_time(rise_hour, rise_minute, s_rise_time_buffer, sizeof(s_rise_time_buffer));
            snprintf(content->grid_bottom_left, sizeof(content->grid_bottom_left), "%s", s_rise_time_buffer);
        } else {
            snprintf(content->grid_bottom_left, sizeof(content->grid_bottom_left), "--:--");
        }

        if (set_hour != SENTINEL_HOUR && set_minute != SENTINEL_MIN) {
            msgproc_format_time(set_hour, set_minute, s_set_time_buffer, sizeof(s_set_time_buffer));
            snprintf(content->grid_bottom_right, sizeof(content->grid_bottom_right), "%s", s_set_time_buffer);
        } else {
            snprintf(content->grid_bottom_right, sizeof(content->grid_bottom_right), "--:--");
        }
    } else {
        // For other bodies (moons, future constellations): show azimuth and altitude
        snprintf(content->grid_top_left, sizeof(content->grid_top_left), "AZIMUTH");
        snprintf(content->grid_top_right, sizeof(content->grid_top_right), "ALTITUDE");
        
        const char *az_str = msgproc_format_angle((int)azimuth, true);
        snprintf(content->grid_bottom_left, sizeof(content->grid_bottom_left), "%s", az_str);
        
        const char *alt_str = msgproc_format_angle((int)altitude, false);
        snprintf(content->grid_bottom_right, sizeof(content->grid_bottom_right), "%s", alt_str);
    }

    // long_text buffer will be filled by details.c
    content->image_resource_id = resource_id;

    // Determine image type (planets use bitmap, others use PDC)
    content->image_type = (body_id > 8) ? DETAILS_IMAGE_TYPE_PDC : DETAILS_IMAGE_TYPE_BITMAP;

    // Store raw azimuth and altitude for locator functionality
    content->azimuth_deg = (int16_t)azimuth;
    content->altitude_deg = (int16_t)altitude;
    content->illumination_x10 = (int16_t)luminance_x10;

    return true;
}

const char* msgproc_format_time(int hour, int minute, char *buffer, size_t buffer_size) {
    if (hour >= 24 || minute >= 60) {
        return "--:--";
    }

    if (clock_is_24h_style()) {
        // 24-hour format
        snprintf(buffer, buffer_size, "%02d:%02d",
                 hour, minute);
    } else {
        // 12-hour format
        bool is_pm = (hour >= 12);
        int display_hour = hour % 12;
        if (display_hour == 0) display_hour = 12;

        snprintf(buffer, buffer_size, "%d:%02d%s",
                 display_hour, minute, is_pm ? "PM" : "AM");
    }

    return buffer;
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
