#include "tile.h"
#include "../core/theme.h"
#include <cstring>

struct TileData {
  const char *title;
  TileClickHandler cb;
};

static TileData tile_data[8];
static int tile_count = 0;

static void tile_event_cb(lv_event_t *e) {
  TileData *data = (TileData *)lv_event_get_user_data(e);
  if (data && data->cb) data->cb(data->title);
}

lv_obj_t *tile_create(lv_obj_t *parent, int x, int y, const char *icon, const char *title, TileClickHandler cb) {
  if (tile_count >= 8) tile_count = 0;

  tile_data[tile_count].title = title;
  tile_data[tile_count].cb = cb;

  lv_obj_t *box = lv_obj_create(parent);
  lv_obj_set_size(box, 62, 62);
  lv_obj_set_pos(box, x, y);
  theme_apply_panel(box);
  lv_obj_set_style_bg_color(box, COLOR_PANEL_2, 0);
  lv_obj_set_style_border_color(box, COLOR_PRIMARY, 0);
  lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(box, tile_event_cb, LV_EVENT_CLICKED, &tile_data[tile_count]);

  lv_obj_t *icon_label = lv_label_create(box);
  lv_label_set_text(icon_label, icon);
  lv_obj_set_style_text_color(icon_label, COLOR_PRIMARY, 0);
  lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_24, 0);
  lv_obj_align(icon_label, LV_ALIGN_TOP_MID, 0, 1);

  lv_obj_t *name = lv_label_create(box);
  lv_label_set_text(name, title);
  theme_apply_small_label(name, false);
  lv_obj_align(name, LV_ALIGN_BOTTOM_MID, 0, -2);

  tile_count++;
  return box;
}
