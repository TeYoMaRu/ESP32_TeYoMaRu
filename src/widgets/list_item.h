#ifndef LIST_ITEM_H
#define LIST_ITEM_H

#include <lvgl.h>

/*
  สร้างรายการเมนูมาตรฐาน

  ตัวอย่าง:
  WiFi
  Bluetooth
  Brightness
  About
*/
lv_obj_t *ui_create_list_item(
  lv_obj_t *parent,
  const char *icon,
  const char *title,
  const char *description,
  lv_event_cb_t event_cb,
  void *user_data = nullptr
);

#endif