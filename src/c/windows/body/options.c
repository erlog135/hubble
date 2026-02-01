#include "options.h"
#include "locator.h"
#include "details.h"
#include "../favorites.h"
#include "../../style.h"
#include "../../utils/settings.h"

static ActionMenu *s_menu;
static ActionMenuLevel *s_root;

static void prv_destroy_menu(void) {
  if (s_root) {
    action_menu_hierarchy_destroy(s_root, NULL, NULL);
    s_root = NULL;
  }
  s_menu = NULL;
}

static void prv_on_favorite(ActionMenu *menu, const ActionMenuItem *action, void *context) {
  (void)menu;
  (void)action;
  (void)context;

  // Get current details content and toggle favorite status
  const DetailsContent *content = details_get_current_content();
  if (content && content->body_id >= 0) {
    int body_id = content->body_id;
    LocalSettings *settings = settings_get();
    bool was_favorited = (settings->favorites & (1 << body_id)) != 0;

      // Toggle the favorite bit
      if (was_favorited) {
        settings->favorites &= ~(1 << body_id);
      } else {
        settings->favorites |= (1 << body_id);
      }

      // Save settings
      settings_save();

      // If unfavoriting and we came from favorites menu, remove it from stack
      if (was_favorited) {
        Window *favorites_window = favorites_get_window();
        if (favorites_window && window_stack_contains_window(favorites_window)) {
          window_stack_remove(favorites_window, true);
        }
      }

      // Action menu will be automatically dismissed, returning to body details
  }
}

static void prv_on_locate(ActionMenu *menu, const ActionMenuItem *action, void *context) {
  (void)menu;
  (void)action;
  (void)context;

  // Get current details content and set locator target
  const DetailsContent *content = details_get_current_content();
  if (content) {
    locator_set_target(content->altitude_deg, content->azimuth_deg);
  }

  locator_show();
}

static void prv_on_refresh(ActionMenu *menu, const ActionMenuItem *action, void *context) {
  (void)menu;
  (void)action;
  (void)context;

  // Get current body ID
  const DetailsContent *content = details_get_current_content();
  if (content && content->body_id >= 0) {
    int body_id = content->body_id;
    
    // Remove the details window from the stack
    details_hide();
    
    // Request the body data again and show the window
    details_show_body(body_id);
  }
}


static void prv_on_close(ActionMenu *menu, const ActionMenuItem *performed_action, void *context) {
  (void)menu;
  (void)performed_action;
  (void)context;
  prv_destroy_menu();
}

void options_menu_show(void) {
  if (s_menu) {
    return;
  }

  const Layout *layout = layout_get();
  s_root = action_menu_level_create(3);
  action_menu_level_add_action(s_root, "Locate", prv_on_locate, NULL);
  action_menu_level_add_action(s_root, "Refresh", prv_on_refresh, NULL);

  // Determine favorite action text based on current status
  const DetailsContent *content = details_get_current_content();
  const char *favorite_text = "Favorite";
  if (content && content->body_id >= 0 && (settings_get()->favorites & (1 << content->body_id))) {
    favorite_text = "Unfavorite";
  }
  action_menu_level_add_action(s_root, favorite_text, prv_on_favorite, NULL);

  ActionMenuConfig config = (ActionMenuConfig){
      .root_level = s_root,
      .colors = {
          .background = layout->highlight,
          .foreground = layout->highlight_foreground,
      },
      .did_close = prv_on_close,
  };

  s_menu = action_menu_open(&config);
}

void options_menu_deinit(void) {
  if (s_menu) {
    action_menu_close(s_menu, false);
    s_menu = NULL;
  }
  prv_destroy_menu();
}