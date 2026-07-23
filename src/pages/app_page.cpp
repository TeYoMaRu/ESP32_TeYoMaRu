#include "app_page.h"

#include "../app/navigation.h"
#include "../core/theme.h"
#include "../widgets/statusbar.h"

#include <lvgl.h>

/* ======================================================
   Event ปุ่ม Back
====================================================== */

static void back_cb(lv_event_t *event) {
  LV_UNUSED(event);

  nav_open_desktop();
}

/* ======================================================
   สร้างโครงหน้ามาตรฐาน

   screen_out:
   - ใช้ส่ง Screen หลักกลับไปให้หน้า WiFi/Bluetooth
   - เพื่อให้เรียก lv_scr_load(screen) ภายหลังได้

   ค่าที่คืนกลับ:
   - พื้นที่ Content สำหรับวางเนื้อหาของแต่ละแอป
====================================================== */

lv_obj_t *app_page_create_shell(
  const char *title,
  lv_obj_t **screen_out
) {
  /* สร้าง Screen */
  lv_obj_t *screen = lv_obj_create(NULL);
  theme_apply_screen(screen);

  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  /* ส่ง Screen กลับไปให้ผู้เรียก */
  if (screen_out) {
    *screen_out = screen;
  }

  /* Status Bar */
  statusbar_create(screen);

  /* ----------------------------------------------------
     ชื่อหน้า
  ---------------------------------------------------- */

  lv_obj_t *header = lv_label_create(screen);

  lv_label_set_text(
    header,
    title ? title : "Application"
  );

  theme_apply_label(header, true);

  lv_obj_set_width(header, 250);
  lv_obj_set_style_text_align(
    header,
    LV_TEXT_ALIGN_CENTER,
    0
  );

  lv_obj_align(
    header,
    LV_ALIGN_TOP_MID,
    0,
    39
  );

  /* ----------------------------------------------------
     ปุ่ม Back
  ---------------------------------------------------- */

  lv_obj_t *back_button = lv_btn_create(screen);

  lv_obj_set_size(
    back_button,
    82,
    34
  );

  lv_obj_align(
    back_button,
    LV_ALIGN_TOP_LEFT,
    12,
    33
  );

  lv_obj_set_style_bg_color(
    back_button,
    COLOR_PANEL_2,
    0
  );

  lv_obj_set_style_border_width(
    back_button,
    1,
    0
  );

  lv_obj_set_style_border_color(
    back_button,
    COLOR_BORDER,
    0
  );

  lv_obj_set_style_radius(
    back_button,
    8,
    0
  );

  lv_obj_add_event_cb(
    back_button,
    back_cb,
    LV_EVENT_CLICKED,
    NULL
  );

  lv_obj_t *back_label = lv_label_create(back_button);

  lv_label_set_text(
    back_label,
    LV_SYMBOL_LEFT " Back"
  );

  lv_obj_set_style_text_color(
    back_label,
    COLOR_TEXT,
    0
  );

  lv_obj_set_style_text_font(
    back_label,
    &lv_font_montserrat_14,
    0
  );

  lv_obj_center(back_label);

  /* ----------------------------------------------------
     พื้นที่ Content กลางหน้า

     หน้า WiFi / Bluetooth / Settings
     จะนำ Object ไปวางในพื้นที่นี้
  ---------------------------------------------------- */

  lv_obj_t *content = lv_obj_create(screen);

  lv_obj_set_size(
    content,
    456,
    228
  );

  lv_obj_align(
    content,
    LV_ALIGN_BOTTOM_MID,
    0,
    -8
  );

  lv_obj_set_style_bg_opa(
    content,
    LV_OPA_TRANSP,
    0
  );

  lv_obj_set_style_border_width(
    content,
    0,
    0
  );

  lv_obj_set_style_pad_all(
    content,
    0,
    0
  );

  lv_obj_clear_flag(
    content,
    LV_OBJ_FLAG_SCROLLABLE
  );

  return content;
}

/* ======================================================
   หน้า Coming Soon

   ใช้กับเมนูที่ยังไม่มีหน้าจริง เช่น:
   - Files
   - System
   - Settings
   - About
   - Power
====================================================== */

void app_page_create(const char *title) {
  lv_obj_t *screen = NULL;

  lv_obj_t *content = app_page_create_shell(
    title,
    &screen
  );

  if (!screen || !content) {
    return;
  }

  /* กล่อง Coming Soon */
  lv_obj_t *panel = lv_obj_create(content);

  lv_obj_set_size(
    panel,
    270,
    95
  );

  lv_obj_align(
    panel,
    LV_ALIGN_CENTER,
    0,
    0
  );

  theme_apply_panel(panel);

  /* ข้อความ */
  lv_obj_t *message = lv_label_create(panel);

  lv_label_set_text(
    message,
    "Coming Soon"
  );

  theme_apply_label(
    message,
    false
  );

  lv_obj_center(message);

  /* โหลดหน้า */
  lv_scr_load(screen);
}