#include "tile.h"
#include "../core/theme.h"
#include <cstring>
#include <cstdlib>

/* ======================================================
   tile.cpp - TeYoMaRu OS

   รูปแบบ Tile ใหม่:
   - Container ขนาดใหญ่ครอบทั้งไอคอนและชื่อ
   - กล่องไอคอนอยู่ด้านบน
   - ชื่อ App อยู่ด้านล่าง
   - กดได้ทั้งกล่องและชื่อ
   - ใช้ LVGL Symbol ลดการใช้ RAM

   Memory:
   ใช้ lv_mem_alloc() สร้าง TileData แยกทุก Tile
   และคืน Memory เมื่อ LVGL ลบ Object
====================================================== */

/* ======================================================
   ค่าปรับ Layout ของ Tile
   แก้ขนาด Tile ทั้งหมดจากส่วนนี้
====================================================== */

/* ขนาด Container ที่ครอบไอคอนและชื่อ */
#define TILE_CONTAINER_W  84
#define TILE_CONTAINER_H  98

/* ขนาดกล่องไอคอน */
#define TILE_ICON_BOX_W   72
#define TILE_ICON_BOX_H   72

/* ตำแหน่งกล่องไอคอนภายใน Container */
#define TILE_ICON_BOX_X    6
#define TILE_ICON_BOX_Y    0

/* ตำแหน่งชื่อ App ใต้กล่อง */
#define TILE_TITLE_Y      76

/* มุมโค้งของกล่องไอคอน */
#define TILE_RADIUS       12

/* ======================================================
   ข้อมูลประจำ Tile
====================================================== */

struct TileData {
  const char *title;
  TileClickHandler cb;
};

/* ======================================================
   Event เมื่อกด Tile
====================================================== */

static void tile_event_cb(lv_event_t *e) {
  TileData *data = (TileData *)lv_event_get_user_data(e);

  if (data && data->cb) {
    data->cb(data->title);
  }
}

/* ======================================================
   คืน Memory เมื่อ Tile ถูกลบ
====================================================== */

static void tile_delete_cb(lv_event_t *e) {
  TileData *data = (TileData *)lv_event_get_user_data(e);

  if (data) {
    lv_mem_free(data);
  }
}

/* ======================================================
   สร้าง Tile
====================================================== */

lv_obj_t *tile_create(
  lv_obj_t *parent,
  int x,
  int y,
  const char *icon,
  const char *title,
  TileClickHandler cb
) {
  /* --------------------------------------------------
     สร้างข้อมูลประจำ Tile
  -------------------------------------------------- */

  TileData *data = (TileData *)lv_mem_alloc(sizeof(TileData));

  if (!data) {
    return NULL;
  }

  data->title = title;
  data->cb = cb;

  /* --------------------------------------------------
     Container หลัก

     Container นี้ครอบ:
     - กล่องไอคอน
     - ชื่อ App

     การกดบริเวณใดก็ได้ใน Container จะเปิด App
  -------------------------------------------------- */

  lv_obj_t *container = lv_obj_create(parent);

  lv_obj_remove_style_all(container);

  lv_obj_set_size(
    container,
    TILE_CONTAINER_W,
    TILE_CONTAINER_H
  );

  lv_obj_set_pos(
    container,
    x,
    y
  );

  lv_obj_set_style_bg_opa(
    container,
    LV_OPA_TRANSP,
    0
  );

  lv_obj_clear_flag(
    container,
    LV_OBJ_FLAG_SCROLLABLE
  );

  lv_obj_add_flag(
    container,
    LV_OBJ_FLAG_CLICKABLE
  );

  /* Event กด Tile */
  lv_obj_add_event_cb(
    container,
    tile_event_cb,
    LV_EVENT_CLICKED,
    data
  );

  /* Event คืน Memory */
  lv_obj_add_event_cb(
    container,
    tile_delete_cb,
    LV_EVENT_DELETE,
    data
  );

  /* --------------------------------------------------
     กล่องไอคอน

     วิธีปรับขนาด:
     แก้ TILE_ICON_BOX_W และ TILE_ICON_BOX_H ด้านบน

     วิธีปรับตำแหน่ง:
     แก้ TILE_ICON_BOX_X และ TILE_ICON_BOX_Y ด้านบน
  -------------------------------------------------- */

  lv_obj_t *icon_box = lv_obj_create(container);

  lv_obj_set_size(
    icon_box,
    TILE_ICON_BOX_W,
    TILE_ICON_BOX_H
  );

  lv_obj_set_pos(
    icon_box,
    TILE_ICON_BOX_X,
    TILE_ICON_BOX_Y
  );

  /* พื้นหลังน้ำเงินเข้ม */
  lv_obj_set_style_bg_color(
    icon_box,
    COLOR_PANEL_2,
    0
  );

  lv_obj_set_style_bg_opa(
    icon_box,
    LV_OPA_COVER,
    0
  );

  /* ขอบสีน้ำเงิน */
  lv_obj_set_style_border_color(
    icon_box,
    COLOR_PRIMARY,
    0
  );

  lv_obj_set_style_border_width(
    icon_box,
    1,
    0
  );

  lv_obj_set_style_border_opa(
    icon_box,
    LV_OPA_90,
    0
  );

  lv_obj_set_style_radius(
    icon_box,
    TILE_RADIUS,
    0
  );

  /* เงาสีน้ำเงินอ่อน */
  lv_obj_set_style_shadow_color(
    icon_box,
    COLOR_PRIMARY,
    0
  );

  lv_obj_set_style_shadow_width(
    icon_box,
    8,
    0
  );

  lv_obj_set_style_shadow_opa(
    icon_box,
    LV_OPA_30,
    0
  );

  /* ตอนกด ให้พื้นหลังเข้มขึ้น */
  lv_obj_set_style_bg_color(
    icon_box,
    COLOR_PANEL,
    LV_STATE_PRESSED
  );

  lv_obj_clear_flag(
    icon_box,
    LV_OBJ_FLAG_SCROLLABLE
  );

  /*
     icon_box ไม่รับ Click เอง
     เพื่อให้ Event ไปที่ Container หลัก
  */
  lv_obj_clear_flag(
    icon_box,
    LV_OBJ_FLAG_CLICKABLE
  );

  /* --------------------------------------------------
     Icon ด้านในกล่อง
  -------------------------------------------------- */

  lv_obj_t *icon_label = lv_label_create(icon_box);

  lv_label_set_text(
    icon_label,
    icon
  );

  lv_obj_set_style_text_color(
    icon_label,
    COLOR_PRIMARY,
    0
  );

  /*
     ขนาด Icon

     หาก Icon ใหญ่เกินไป:
     เปลี่ยน 28 เป็น 24

     หาก Icon เล็กเกินไป:
     เปลี่ยน 28 เป็น 32
     แต่ต้องเปิด Font ใน lv_conf.h ก่อน
  */
  lv_obj_set_style_text_font(
    icon_label,
    &lv_font_montserrat_28,
    0
  );

  lv_obj_center(icon_label);

  lv_obj_clear_flag(
    icon_label,
    LV_OBJ_FLAG_CLICKABLE
  );

  /* --------------------------------------------------
     ชื่อ App ใต้กล่องไอคอน
  -------------------------------------------------- */

  lv_obj_t *name_label = lv_label_create(container);

  lv_label_set_text(
    name_label,
    title
  );

  lv_obj_set_width(
    name_label,
    TILE_CONTAINER_W
  );

  lv_obj_set_style_text_align(
    name_label,
    LV_TEXT_ALIGN_CENTER,
    0
  );

  lv_obj_set_style_text_color(
    name_label,
    lv_color_white(),
    0
  );

  /*
     ขนาดชื่อ App

     หากชื่อใหญ่เกินไป:
     เปลี่ยน 14 เป็น 12
  */
  lv_obj_set_style_text_font(
    name_label,
    &lv_font_montserrat_14,
    0
  );

  /*
     ตำแหน่งชื่อ App

     x = 0
     y = TILE_TITLE_Y

     แก้ TILE_TITLE_Y ด้านบน:
     - ค่าน้อยลง = ขยับขึ้น
     - ค่ามากขึ้น = ขยับลง
  */
  lv_obj_set_pos(
    name_label,
    0,
    TILE_TITLE_Y
  );

  lv_obj_clear_flag(
    name_label,
    LV_OBJ_FLAG_CLICKABLE
  );

  return container;
}
