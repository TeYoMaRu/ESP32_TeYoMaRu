#include "widgets.h"
#include "../core/theme.h"
#include <cstring>

/* =========================================================
   เก็บข้อมูลของ Tile แต่ละอัน

   เหตุผลที่ต้องใช้ struct:
   LVGL ส่ง user_data เข้า event ได้
   เราเลยเก็บ title + handler ไว้ด้วยกัน
   ========================================================= */
struct TileEventData {
  const char *title;
  TileClickHandler handler;
};

/*
   ใช้ static เพราะข้อมูลต้องอยู่ตลอดเวลา
   ห้ามสร้างเป็น local variable แล้วส่งให้ event
   เพราะพอฟังก์ชันจบ ข้อมูลจะหาย
*/
static TileEventData tile_scan = {"SCAN", NULL};
static TileEventData tile_wifi = {"WIFI", NULL};
static TileEventData tile_sys  = {"SYS",  NULL};

/*
   เลือกกล่องข้อมูลให้ตรงกับ title
*/
static TileEventData *pick_tile_data(const char *title) {
  if (strcmp(title, "SCAN") == 0) return &tile_scan;
  if (strcmp(title, "WIFI") == 0) return &tile_wifi;
  return &tile_sys;
}

/*
   Event ตอนกด Tile
*/
static void tile_event_cb(lv_event_t *e) {
  TileEventData *data = (TileEventData *)lv_event_get_user_data(e);

  if (!data) return;
  if (!data->handler) return;

  data->handler(data->title);
}

/* =========================================================
   สร้าง Panel / Card
   ========================================================= */
lv_obj_t *ui_create_panel(
  lv_obj_t *parent,
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h
) {
  lv_obj_t *panel = lv_obj_create(parent);

  lv_obj_set_size(panel, w, h);
  lv_obj_set_pos(panel, x, y);

  lv_obj_set_style_radius(panel, 18, 0);
  lv_obj_set_style_bg_color(panel, C_CARD, 0);
  lv_obj_set_style_border_width(panel, 1, 0);
  lv_obj_set_style_border_color(panel, lv_color_hex(0x1E40AF), 0);

  return panel;
}

/* =========================================================
   สร้าง Tile เมนูแบบกดได้
   ========================================================= */
lv_obj_t *ui_create_tile(
  lv_obj_t *parent,
  int16_t x,
  int16_t y,
  const char *title,
  const char *desc,
  TileClickHandler handler
) {
  lv_obj_t *tile = lv_obj_create(parent);

  lv_obj_set_size(tile, 130, 82);
  lv_obj_set_pos(tile, x, y);

  // ทำให้ Tile กดได้
  lv_obj_add_flag(tile, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_set_style_radius(tile, 14, 0);
  lv_obj_set_style_bg_color(tile, lv_color_hex(0x111827), 0);
  lv_obj_set_style_border_width(tile, 1, 0);
  lv_obj_set_style_border_color(tile, C_BLUE, 0);
  lv_obj_set_style_shadow_width(tile, 8, 0);
  lv_obj_set_style_shadow_color(tile, C_BLUE_DARK, 0);

  // สีตอนกด
  lv_obj_set_style_bg_color(tile, lv_color_hex(0x164E63), LV_STATE_PRESSED);
  lv_obj_set_style_border_color(tile, lv_color_hex(0x67E8F9), LV_STATE_PRESSED);

  lv_obj_t *label_title = lv_label_create(tile);
  lv_label_set_text(label_title, title);
  lv_obj_set_style_text_color(label_title, C_BLUE, 0);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_18, 0);
  lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 8, 8);

  lv_obj_t *label_desc = lv_label_create(tile);
  lv_label_set_text(label_desc, desc);
  lv_obj_set_style_text_color(label_desc, C_MUTED, 0);
  lv_obj_set_style_text_font(label_desc, &lv_font_montserrat_14, 0);
  lv_obj_align(label_desc, LV_ALIGN_TOP_LEFT, 8, 40);

  TileEventData *data = pick_tile_data(title);
  data->title = title;
  data->handler = handler;

  lv_obj_add_event_cb(tile, tile_event_cb, LV_EVENT_CLICKED, data);

  return tile;
}