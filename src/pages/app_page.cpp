#include "app_page.h"
#include "desktop.h"
#include "../core/theme.h"
#include "../widgets/statusbar.h"
#include <lvgl.h>

static void back_cb(lv_event_t *e) {
  (void)e;
  desktop_create();
}

void app_page_create(const char *title) {
  lv_obj_t *scr = lv_obj_create(NULL);
  theme_apply_screen(scr);
  statusbar_create(scr);

  lv_obj_t *head = lv_label_create(scr);
  lv_label_set_text(head, title);
  theme_apply_label(head, true);
  lv_obj_align(head, LV_ALIGN_TOP_MID, 0, 40);

  lv_obj_t *panel = lv_obj_create(scr);
  lv_obj_set_size(panel, 270, 95);
  lv_obj_align(panel, LV_ALIGN_CENTER, 0, 10);
  theme_apply_panel(panel);

  lv_obj_t *msg = lv_label_create(panel);
  lv_label_set_text(msg, "Coming Soon");
  theme_apply_label(msg, false);
  lv_obj_center(msg);

  lv_obj_t *btn = lv_btn_create(scr);
  lv_obj_set_size(btn, 90, 32);
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_bg_color(btn, COLOR_PRIMARY, 0);
  lv_obj_add_event_cb(btn, back_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *bt = lv_label_create(btn);
  lv_label_set_text(bt, "Back");
  lv_obj_set_style_text_color(bt, COLOR_BG, 0);
  lv_obj_center(bt);

  lv_scr_load(scr);
}
