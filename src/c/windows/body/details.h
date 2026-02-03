/**
 * Body details window showing a hero PDC image, summary stats, and long-form
 * description text inside a scroll layer.
 */
#pragma once

#include <pebble.h>

typedef enum {
  DETAILS_IMAGE_TYPE_PDC,
  DETAILS_IMAGE_TYPE_BITMAP,
} DetailsImageType;

typedef struct {
  const char *title_text;
  const char *detail_text;
  const char *grid_top_left;
  const char *grid_top_right;
  const char *grid_bottom_left;
  const char *grid_bottom_right;
  char long_text[128];  // Buffer for dynamically formatted long text
  uint32_t image_resource_id;  // RESOURCE_ID_*
  DetailsImageType image_type;
  int16_t azimuth_deg;  // Azimuth in degrees (0-360)
  int16_t altitude_deg; // Altitude in degrees (-90 to 90)
  int16_t illumination_x10; // Illumination as magnitude * 10 (-256 to 255)
  int body_id;  // Body ID for favoriting (-1 if not applicable)
} DetailsContent;

void details_init(void);
void details_deinit(void);

// Passing NULL uses the default static content.
void details_show(const DetailsContent *content);

// Show details for a specific body by requesting data from the phone
void details_show_body(int body_id);

void details_hide(void);

// Get the current details content (for use by options menu)
const DetailsContent* details_get_current_content(void);