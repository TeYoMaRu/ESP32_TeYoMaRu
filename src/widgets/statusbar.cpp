#include "statusbar.h"
#include "../core/theme.h"

lv_obj_t *statusbar_create(lv_obj_t *parent) {
  lv_obj_t *bar = lv_obj_create(parent);
  lv_obj_set_size(bar, LV_PCT(100), 26);   // เต็มความกว้างจอ (480px) แทนที่จะ fix ไว้ที่ 320
  lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(bar, COLOR_BG, 0);
  lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_set_style_radius(bar, 0, 0);
  lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *time = lv_label_create(bar);
  lv_label_set_text(time, "09:41");
  theme_apply_small_label(time, false);
  lv_obj_align(time, LV_ALIGN_LEFT_MID, 8, 0);

  lv_obj_t *icons = lv_label_create(bar);
  lv_label_set_text(icons, "WiFi  BT  78%");
  theme_apply_small_label(icons, false);
  lv_obj_set_style_text_color(icons, COLOR_PRIMARY, 0);
  lv_obj_align(icons, LV_ALIGN_RIGHT_MID, -8, 0);

  return bar;
}
