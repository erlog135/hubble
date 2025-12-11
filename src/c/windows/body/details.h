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
  const char *long_text;
  uint32_t image_resource_id;  // RESOURCE_ID_*
  DetailsImageType image_type;
} DetailsContent;

void details_init(void);
void details_deinit(void);

// Passing NULL uses the default static content.
void details_show(const DetailsContent *content);
void details_hide(void);