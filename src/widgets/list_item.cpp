#include "list_item.h"
#include "../core/theme.h"

/* =========================================================
   สร้างรายการเมนูมาตรฐาน
========================================================= */

lv_obj_t *ui_create_list_item(
  lv_obj_t *parent,
  const char *icon,
  const char *title,
  const char *description,
  lv_event_cb_t event_cb,
  void *user_data
) {
  if (!parent || !title) {
    return nullptr;
  }

  lv_obj_t *item = lv_btn_create(parent);

  lv_obj_set_width(item, lv_pct(100));
  lv_obj_set_height(item, 62);

  lv_obj_set_style_bg_color(
    item,
    COLOR_PANEL_2,
    0
  );

  lv_obj_set_style_bg_color(
    item,
    COLOR_PANEL,
    LV_STATE_PRESSED
  );

  lv_obj_set_style_border_width(item, 1, 0);
  lv_obj_set_style_border_color(
    item,
    COLOR_BORDER,
    0
  );

  lv_obj_set_style_radius(item, 10, 0);
  lv_obj_set_style_shadow_width(item, 0, 0);

  lv_obj_set_style_pad_left(item, 12, 0);
  lv_obj_set_style_pad_right(item, 12, 0);

  /* ไอคอน */
  lv_obj_t *icon_label = nullptr;

  if (icon && icon[0] != '\0') {
    icon_label = lv_label_create(item);

    lv_label_set_text(icon_label, icon);

    lv_obj_set_style_text_color(
      icon_label,
      COLOR_PRIMARY,
      0
    );

    lv_obj_set_style_text_font(
      icon_label,
      &lv_font_montserrat_18,
      0
    );

    lv_obj_align(
      icon_label,
      LV_ALIGN_LEFT_MID,
      0,
      0
    );
  }

  /* ชื่อรายการ */
  lv_obj_t *title_label = lv_label_create(item);

  lv_label_set_text(title_label, title);

  lv_obj_set_style_text_color(
    title_label,
    COLOR_TEXT,
    0
  );

  lv_obj_set_style_text_font(
    title_label,
    &lv_font_montserrat_16,
    0
  );

  lv_obj_align(
    title_label,
    LV_ALIGN_LEFT_MID,
    icon_label ? 35 : 0,
    description ? -10 : 0
  );

  /* คำอธิบาย */
  if (description && description[0] != '\0') {
    lv_obj_t *description_label =
      lv_label_create(item);

    lv_label_set_text(
      description_label,
      description
    );

    lv_obj_set_style_text_color(
      description_label,
      COLOR_MUTED,
      0
    );

    lv_obj_set_style_text_font(
      description_label,
      &lv_font_montserrat_12,
      0
    );

    lv_obj_align(
      description_label,
      LV_ALIGN_LEFT_MID,
      icon_label ? 35 : 0,
      13
    );
  }

  /* ลูกศรด้านขวา */
  lv_obj_t *arrow_label = lv_label_create(item);

  lv_label_set_text(
    arrow_label,
    LV_SYMBOL_RIGHT
  );

  lv_obj_set_style_text_color(
    arrow_label,
    COLOR_MUTED,
    0
  );

  lv_obj_align(
    arrow_label,
    LV_ALIGN_RIGHT_MID,
    0,
    0
  );

  /* Event ตอนกด */
  if (event_cb) {
    lv_obj_add_event_cb(
      item,
      event_cb,
      LV_EVENT_CLICKED,
      user_data
    );
  }

  return item;
}