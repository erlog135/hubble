#include "options.h"
#include "locator.h"
#include "../../style.h"

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
  // Placeholder: no-op for now.
  (void)menu;
  (void)action;
  (void)context;
}

static void prv_on_locate(ActionMenu *menu, const ActionMenuItem *action, void *context) {
  (void)menu;
  (void)action;
  (void)context;
  locator_show();
}

static void prv_on_refresh(ActionMenu *menu, const ActionMenuItem *action, void *context) {
  // Placeholder: no-op for now.
  (void)menu;
  (void)action;
  (void)context;
}

static void prv_on_hide(ActionMenu *menu, const ActionMenuItem *action, void *context) {
  // Placeholder: no-op for now.
  (void)menu;
  (void)action;
  (void)context;
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
  s_root = action_menu_level_create(4);
  action_menu_level_add_action(s_root, "Locate", prv_on_locate, NULL);
  action_menu_level_add_action(s_root, "Refresh", prv_on_refresh, NULL);
  action_menu_level_add_action(s_root, "Favorite", prv_on_favorite, NULL);
  action_menu_level_add_action(s_root, "Hide", prv_on_hide, NULL);

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