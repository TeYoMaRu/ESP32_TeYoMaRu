#ifndef WIDGETS_H
#define WIDGETS_H

#include <lvgl.h>

/* =========================================================
   widgets.h

   รวม Widget ที่ใช้ซ้ำใน TeYoMaRu OS
   ========================================================= */

// ชนิดของฟังก์ชัน callback ตอนกด Tile
typedef void (*TileClickHandler)(const char *title);

// สร้าง Panel / Card พื้นฐาน
lv_obj_t *ui_create_panel(
  lv_obj_t *parent,
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h
);

// สร้าง Tile เมนูแบบกดได้
lv_obj_t *ui_create_tile(
  lv_obj_t *parent,
  int16_t x,
  int16_t y,
  const char *title,
  const char *desc,
  TileClickHandler handler
);

#endif