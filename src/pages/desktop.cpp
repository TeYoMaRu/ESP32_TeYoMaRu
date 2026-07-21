#include "desktop.h"
#include "../core/theme.h"
#include "../widgets/statusbar.h"
#include "../widgets/tile.h"
#include "../app/navigation.h"
#include <lvgl.h>

/* ======================================================
   TeYoMaRu OS - Desktop / Home Screen
   สำหรับจอแนวนอน 480 x 320

   รูปแบบหน้าจอ:
   - Status Bar ด้านบน
   - แอป 4 คอลัมน์ x 2 แถว
   - กล่องไอคอนสีน้ำเงิน
   - ชื่อแอปอยู่ใต้กล่อง
   - จุดแสดงหน้าด้านล่าง

   วิธีปรับ Layout:
   - ปรับตำแหน่งทั้งชุดจาก TILE_START_X / TILE_START_Y
   - ปรับระยะห่างจาก TILE_GAP_X / TILE_GAP_Y
   - ปรับขนาด Tile ใน tile.cpp
====================================================== */

/* ======================================================
   ค่าตำแหน่ง Desktop
   แก้ Layout หลักจากส่วนนี้ได้เลย
====================================================== */

/* จุดเริ่มต้นของ Tile คอลัมน์แรก */
#define TILE_START_X  47

/* จุดเริ่มต้นของ Tile แถวแรก */
#define TILE_START_Y  58

/* ระยะห่างแนวนอนระหว่าง Tile */
#define TILE_GAP_X    99

/* ระยะห่างแนวตั้งระหว่าง Tile */
#define TILE_GAP_Y    109

/* ======================================================
   Event เมื่อกด Tile
====================================================== */

static void on_tile_click(const char *title) {
  nav_open_app(title);
}

/* ======================================================
   สร้างจุดบอกหน้าด้านล่าง
====================================================== */

static void create_page_indicator(lv_obj_t *parent) {
  /*
     จุดซ้ายและขวาเป็นจุดเล็กสีเข้ม
     จุดกลางเป็นหน้าปัจจุบัน สีขาว

     หากต้องการขยับขึ้น/ลง:
     แก้ค่า y จาก -12
     - ค่าติดลบมากขึ้น = ขยับขึ้น
     - ค่าติดลบน้อยลง = ขยับลง
  */

  lv_obj_t *dot_left = lv_obj_create(parent);
  lv_obj_remove_style_all(dot_left);
  lv_obj_set_size(dot_left, 6, 6);
  lv_obj_set_style_radius(dot_left, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(dot_left, COLOR_PRIMARY, 0);
  lv_obj_set_style_bg_opa(dot_left, LV_OPA_COVER, 0);
  lv_obj_align(dot_left, LV_ALIGN_BOTTOM_MID, -18, -12);

  lv_obj_t *dot_center = lv_obj_create(parent);
  lv_obj_remove_style_all(dot_center);
  lv_obj_set_size(dot_center, 9, 9);
  lv_obj_set_style_radius(dot_center, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(dot_center, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(dot_center, LV_OPA_COVER, 0);
  lv_obj_align(dot_center, LV_ALIGN_BOTTOM_MID, 0, -10);

  lv_obj_t *dot_right = lv_obj_create(parent);
  lv_obj_remove_style_all(dot_right);
  lv_obj_set_size(dot_right, 6, 6);
  lv_obj_set_style_radius(dot_right, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(dot_right, COLOR_PRIMARY, 0);
  lv_obj_set_style_bg_opa(dot_right, LV_OPA_COVER, 0);
  lv_obj_align(dot_right, LV_ALIGN_BOTTOM_MID, 18, -12);
}

/* ======================================================
   สร้างหน้า Desktop
====================================================== */

void desktop_create() {
  /* สร้าง Screen หลัก */
  lv_obj_t *scr = lv_obj_create(NULL);
  theme_apply_screen(scr);

  /* ป้องกันหน้าเลื่อน */
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  /* Status Bar ด้านบน */
  statusbar_create(scr);

  /*
     ไม่แสดงคำว่า TeYoMaRu OS กลางจอ
     เพื่อให้ Layout ใกล้รูปตัวอย่าง และมีพื้นที่ Tile มากขึ้น
  */

  /* ====================================================
     แถวที่ 1

     ตำแหน่งคำนวณจาก:
     x = TILE_START_X + (คอลัมน์ * TILE_GAP_X)
     y = TILE_START_Y
  ==================================================== */

  tile_create(
    scr,
    TILE_START_X + (0 * TILE_GAP_X),
    TILE_START_Y,
    LV_SYMBOL_IMAGE,
    "Scan",
    on_tile_click
  );

  tile_create(
    scr,
    TILE_START_X + (1 * TILE_GAP_X),
    TILE_START_Y,
    LV_SYMBOL_WIFI,
    "WiFi",
    on_tile_click
  );

  tile_create(
    scr,
    TILE_START_X + (2 * TILE_GAP_X),
    TILE_START_Y,
    LV_SYMBOL_DIRECTORY,
    "Files",
    on_tile_click
  );

  tile_create(
    scr,
    TILE_START_X + (3 * TILE_GAP_X),
    TILE_START_Y,
    LV_SYMBOL_SETTINGS,
    "System",
    on_tile_click
  );

  /* ====================================================
     แถวที่ 2

     y = TILE_START_Y + TILE_GAP_Y
  ==================================================== */

  tile_create(
    scr,
    TILE_START_X + (0 * TILE_GAP_X),
    TILE_START_Y + TILE_GAP_Y,
    LV_SYMBOL_SETTINGS,
    "Settings",
    on_tile_click
  );

  tile_create(
    scr,
    TILE_START_X + (1 * TILE_GAP_X),
    TILE_START_Y + TILE_GAP_Y,
    LV_SYMBOL_BLUETOOTH,
    "Bluetooth",
    on_tile_click
  );

  tile_create(
    scr,
    TILE_START_X + (2 * TILE_GAP_X),
    TILE_START_Y + TILE_GAP_Y,
    LV_SYMBOL_WARNING,
    "About",
    on_tile_click
  );

  tile_create(
    scr,
    TILE_START_X + (3 * TILE_GAP_X),
    TILE_START_Y + TILE_GAP_Y,
    LV_SYMBOL_POWER,
    "Power",
    on_tile_click
  );

  /* จุดแสดงหน้าด้านล่าง */
  create_page_indicator(scr);

  /* โหลดหน้า Desktop */
  lv_scr_load(scr);
}
