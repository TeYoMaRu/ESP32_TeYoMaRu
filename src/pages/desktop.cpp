#include "desktop.h"
#include "../core/theme.h"
#include "../widgets/statusbar.h"
#include "../widgets/tile.h"
#include "../app/navigation.h"
#include <lvgl.h>

/* ======================================================
   Desktop Screen
   - ไม่ใช้รูป icon เพื่อลด RAM
   - ใช้ตัวอักษรสั้น ๆ แทน icon ก่อน
   - กด Tile แล้วเปิดหน้า Coming Soon ได้ทันที
====================================================== */

static void on_tile_click(const char *title) {
  nav_open_app(title);
}

void desktop_create() {
  lv_obj_t *scr = lv_obj_create(NULL);
  theme_apply_screen(scr);

  statusbar_create(scr);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "TeYoMaRu OS");
  theme_apply_label(title, true);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 32);

  // แถวที่ 1
  tile_create(scr, 22, 65,  "+", "Scan",      on_tile_click);
  tile_create(scr, 94, 65,  "~", "WiFi",      on_tile_click);
  tile_create(scr, 166, 65, "[]", "Files",    on_tile_click);
  tile_create(scr, 238, 65, "#", "System",    on_tile_click);

  // แถวที่ 2
  tile_create(scr, 22, 137, "*", "Settings",  on_tile_click);
  tile_create(scr, 94, 137, "B", "Bluetooth", on_tile_click);
  tile_create(scr, 166, 137,"i", "About",     on_tile_click);
  tile_create(scr, 238, 137,"P", "Power",     on_tile_click);

  lv_obj_t *hint = lv_label_create(scr);
  lv_label_set_text(hint, "Yellow / White / Orange theme");
  theme_apply_small_label(hint, true);
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -5);

  lv_scr_load(scr);
}
