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

  // จอจริงกว้าง 480px (ไม่ใช่ 320px) จึงจัดตำแหน่ง tile ให้อยู่กึ่งกลางแนวนอน
  // บล็อก tile ทั้งหมดกว้าง 4*62 + 3*10 = 278px -> เริ่มที่ x = (480-278)/2 = 101
  // แถวที่ 1
  tile_create(scr, 101, 65,  "+", "Scan",      on_tile_click);
  tile_create(scr, 173, 65,  "~", "WiFi",      on_tile_click);
  tile_create(scr, 245, 65,  "[]", "Files",    on_tile_click);
  tile_create(scr, 317, 65,  "#", "System",    on_tile_click);

  // แถวที่ 2
  tile_create(scr, 101, 137, "*", "Settings",  on_tile_click);
  tile_create(scr, 173, 137, "B", "Bluetooth", on_tile_click);
  tile_create(scr, 245, 137, "i", "About",     on_tile_click);
  tile_create(scr, 317, 137, "P", "Power",     on_tile_click);

  lv_obj_t *hint = lv_label_create(scr);
  lv_label_set_text(hint, "Yellow / White / Orange theme");
  theme_apply_small_label(hint, true);
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -5);

  lv_scr_load(scr);
}
