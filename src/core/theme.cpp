#include "theme.h"

/* ======================================================
   Theme helper functions
   ใช้แต่งหน้าจอ/กล่อง/ตัวหนังสือให้สไตล์ตรงกันทั้งระบบ
====================================================== */

void theme_apply_screen(lv_obj_t *obj) {
  if (!obj) return;
  lv_obj_set_style_bg_color(obj, COLOR_BG, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(obj, 0, 0);
  lv_obj_set_style_radius(obj, 0, 0);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

void theme_apply_panel(lv_obj_t *obj) {
  if (!obj) return;
  lv_obj_set_style_bg_color(obj, COLOR_PANEL, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(obj, 1, 0);
  lv_obj_set_style_border_color(obj, COLOR_BORDER, 0);
  lv_obj_set_style_radius(obj, 14, 0);
  lv_obj_set_style_pad_all(obj, 8, 0);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

void theme_apply_label(lv_obj_t *obj, bool big) {
  if (!obj) return;
  lv_obj_set_style_text_color(obj, COLOR_TEXT, 0);
  lv_obj_set_style_text_font(obj, big ? &lv_font_montserrat_20 : &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
}

void theme_apply_small_label(lv_obj_t *obj, bool center) {
  if (!obj) return;
  lv_obj_set_style_text_color(obj, COLOR_MUTED, 0);
  lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_align(obj, center ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT, 0);
}
