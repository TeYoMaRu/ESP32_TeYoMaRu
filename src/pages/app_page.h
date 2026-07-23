#pragma once

#include <lvgl.h>

/*
  สร้างโครงหน้ามาตรฐานของแอป

  ภายในประกอบด้วย:
  - พื้นหลัง
  - Status Bar
  - ชื่อหน้า
  - ปุ่ม Back
  - พื้นที่ Content

  คืนค่าเป็นพื้นที่ Content
  เพื่อให้แต่ละแอปนำไปใส่เนื้อหาของตัวเอง
*/
lv_obj_t *app_page_create_shell(
  const char *title,
  lv_obj_t **screen_out
);

/*
  หน้า Coming Soon เดิม
  เก็บไว้เพื่อให้เมนูที่ยังไม่ได้สร้างใช้งานต่อได้
*/
void app_page_create(const char *title);