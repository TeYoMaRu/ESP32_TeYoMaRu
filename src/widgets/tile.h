#pragma once

#include <lvgl.h>

/* ======================================================
   ชนิด Callback ของ Tile

   title = ชื่อ App เช่น "Scan", "WiFi", "Files"
====================================================== */

typedef void (*TileClickHandler)(const char *title);

/* ======================================================
   สร้าง Tile บน Desktop

   parent = Screen หรือ Panel ที่ Tile จะอยู่
   x      = ตำแหน่งจากซ้าย
   y      = ตำแหน่งจากบน
   icon   = LVGL Symbol เช่น LV_SYMBOL_WIFI
   title  = ชื่อ App
   cb     = ฟังก์ชันเมื่อกด Tile
====================================================== */

lv_obj_t *tile_create(
  lv_obj_t *parent,
  int x,
  int y,
  const char *icon,
  const char *title,
  TileClickHandler cb
);
