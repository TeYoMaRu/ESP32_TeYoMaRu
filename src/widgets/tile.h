#pragma once
#include <lvgl.h>

typedef void (*TileClickHandler)(const char *title);

lv_obj_t *tile_create(lv_obj_t *parent, int x, int y, const char *icon, const char *title, TileClickHandler cb);
