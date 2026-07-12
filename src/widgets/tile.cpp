#include "tile.h"
#include "../core/theme.h"
#include <cstring>
#include <cstdlib>

/* ======================================================
   tile.cpp  –  TeYoMaRu OS

   แก้ไข: ใช้ lv_mem_alloc() จัดสรร TileData ต่อ tile
   แทน static array เพื่อป้องกัน pointer ถูกทับ
   เมื่อ Desktop ถูกสร้างซ้ำ (กด Back จาก app ต่าง ๆ)
====================================================== */

struct TileData {
  const char *title;
  TileClickHandler cb;
};

/* event callback: อ่าน TileData จาก user_data */
static void tile_event_cb(lv_event_t *e) {
  TileData *data = (TileData *)lv_event_get_user_data(e);
  if (data && data->cb) data->cb(data->title);
}

/*
   คืน memory ของ TileData เมื่อ LVGL ลบ object นั้น
   ป้องกัน memory leak เมื่อ Desktop ถูกสร้างซ้ำ
*/
static void tile_delete_cb(lv_event_t *e) {
  TileData *data = (TileData *)lv_event_get_user_data(e);
  if (data) lv_mem_free(data);
}

lv_obj_t *tile_create(lv_obj_t *parent, int x, int y,
                       const char *icon, const char *title,
                       TileClickHandler cb) {

  /* จัดสรร TileData ใหม่ทุกครั้ง → pointer ไม่ซ้ำกัน */
  TileData *data = (TileData *)lv_mem_alloc(sizeof(TileData));
  if (!data) return NULL;
  data->title = title;
  data->cb    = cb;

  lv_obj_t *box = lv_obj_create(parent);
  lv_obj_set_size(box, 62, 62);
  lv_obj_set_pos(box, x, y);
  theme_apply_panel(box);
  lv_obj_set_style_bg_color(box, COLOR_PANEL_2, 0);
  lv_obj_set_style_border_color(box, COLOR_PRIMARY, 0);
  lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);

  /* ลงทะเบียน event clicked + delete (เพื่อ free memory) */
  lv_obj_add_event_cb(box, tile_event_cb,   LV_EVENT_CLICKED, data);
  lv_obj_add_event_cb(box, tile_delete_cb,  LV_EVENT_DELETE,  data);

  lv_obj_t *icon_label = lv_label_create(box);
  lv_label_set_text(icon_label, icon);
  lv_obj_set_style_text_color(icon_label, COLOR_PRIMARY, 0);
  lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_24, 0);
  lv_obj_align(icon_label, LV_ALIGN_TOP_MID, 0, 1);
  // ป้องกัน label บัง touch ของ box
  lv_obj_clear_flag(icon_label, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(icon_label,  LV_OBJ_FLAG_EVENT_BUBBLE);

  lv_obj_t *name = lv_label_create(box);
  lv_label_set_text(name, title);
  theme_apply_small_label(name, false);
  lv_obj_align(name, LV_ALIGN_BOTTOM_MID, 0, -2);
  lv_obj_clear_flag(name, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(name,  LV_OBJ_FLAG_EVENT_BUBBLE);

  return box;
}
