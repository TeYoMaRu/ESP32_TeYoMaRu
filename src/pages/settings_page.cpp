#include "settings_page.h"
#include "app_page.h"
#include "about_page.h"

#include <Arduino.h>
#include <lvgl.h>
#include <cstring>

#include "../widgets/list_item.h"
#include "../widgets/popup.h"

/* ======================================================
   Event ชั่วคราวของเมนู Settings
====================================================== */



static void setting_item_event_cb(lv_event_t *event) {
    const char *item_name =
        static_cast<const char *>(
            lv_event_get_user_data(event)
        );

    if (!item_name) return;

    Serial.println(item_name);

    if (strcmp(item_name, "About") == 0) {
        about_page_create();
        return;
    }

    popup_show_message(
        item_name,
        "Coming Soon"
    );
}

/* ======================================================
   สร้างหน้า Settings
====================================================== */

void settings_page_create() {
  lv_obj_t *screen = nullptr;

  lv_obj_t *content =
    app_page_create_shell(
      LV_SYMBOL_SETTINGS "  Settings",
      &screen
    );

  if (!screen || !content) {
    return;
  }

  lv_obj_set_flex_flow(
    content,
    LV_FLEX_FLOW_COLUMN
  );

  lv_obj_set_flex_align(
    content,
    LV_FLEX_ALIGN_START,
    LV_FLEX_ALIGN_CENTER,
    LV_FLEX_ALIGN_CENTER
  );

  lv_obj_set_style_pad_all(
    content,
    10,
    0
  );

  lv_obj_set_style_pad_row(
    content,
    8,
    0
  );

  lv_obj_add_flag(content, LV_OBJ_FLAG_SCROLLABLE);
lv_obj_set_scroll_dir(content, LV_DIR_VER);
lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_AUTO);



  ui_create_list_item(
    content,
    LV_SYMBOL_WIFI,
    "WiFi",
    "Network settings",
    setting_item_event_cb,
    (void *)"WiFi"
  );

  ui_create_list_item(
    content,
    LV_SYMBOL_BLUETOOTH,
    "Bluetooth",
    "Bluetooth settings",
    setting_item_event_cb,
    (void *)"Bluetooth"
  );

  ui_create_list_item(
    content,
    LV_SYMBOL_EYE_OPEN,
    "Brightness",
    "Display brightness",
    setting_item_event_cb,
    (void *)"Brightness"
  );

  ui_create_list_item(
    content,
    LV_SYMBOL_EDIT,
    "Theme",
    "System appearance",
    setting_item_event_cb,
    (void *)"Theme"
  );

  ui_create_list_item(
    content,
    LV_SYMBOL_FILE,
    "About",
    "Device information",
    setting_item_event_cb,
    (void *)"About"
  );

  lv_scr_load(screen);
}