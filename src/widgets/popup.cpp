#include "popup.h"
#include "../core/theme.h"

static void close_cb(lv_event_t *e) {
  lv_obj_t *popup = (lv_obj_t *)lv_event_get_user_data(e);
  if (popup) lv_obj_del(popup);
}

void popup_show_message(const char *title, const char *message) {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_t *popup = lv_obj_create(scr);
  lv_obj_set_size(popup, 250, 135);
  lv_obj_center(popup);
  theme_apply_panel(popup);
  lv_obj_set_style_border_color(popup, COLOR_ORANGE, 0);

  lv_obj_t *t = lv_label_create(popup);
  lv_label_set_text(t, title);
  theme_apply_label(t, true);
  lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 4);

  lv_obj_t *m = lv_label_create(popup);
  lv_label_set_text(m, message);
  theme_apply_small_label(m, false);
  lv_obj_align(m, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *btn = lv_btn_create(popup);
  lv_obj_set_size(btn, 74, 28);
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -4);
  lv_obj_set_style_bg_color(btn, COLOR_PRIMARY, 0);
  lv_obj_add_event_cb(btn, close_cb, LV_EVENT_CLICKED, popup);

  lv_obj_t *txt = lv_label_create(btn);
  lv_label_set_text(txt, "OK");
  lv_obj_set_style_text_color(txt, COLOR_BG, 0);
  lv_obj_center(txt);
}
