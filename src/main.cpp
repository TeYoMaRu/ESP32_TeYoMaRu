#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

/* =========================================================
   TeYoMaRu OS - LVGL Starter + Desktop
   ESP32 DevKit V1 + ILI9488 SPI 480x320

   ไฟล์นี้ทำอะไร:
   1) เปิดจอ TFT
   2) เปิด LVGL
   3) แสดงหน้า Boot Screen
   4) เมื่อโหลดครบ 100% จะเปลี่ยนไปหน้า Desktop
   ========================================================= */

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320

TFT_eSPI tft = TFT_eSPI();

/* 
   Buffer สำหรับ LVGL
   ใช้ 480 x 40 เพราะ ESP32 RAM จำกัด
   ไม่ควรใช้เต็มจอ 480x320
*/
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_WIDTH * 40];

// object ของหน้า boot
static lv_obj_t *boot_bar;
static lv_obj_t *percent_label;
static lv_obj_t *status_label;
static lv_timer_t *boot_timer;
static int boot_percent = 0;

/* =========================================================
   ส่งภาพจาก LVGL ไปที่จอ TFT จริง
   ========================================================= */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/* =========================================================
   สร้าง Tile / ปุ่มบนหน้า Desktop
   ========================================================= */
lv_obj_t *create_tile(lv_obj_t *parent, int x, int y, const char *title, const char *desc) {
  // สร้างกล่อง Tile
  lv_obj_t *tile = lv_obj_create(parent);
  lv_obj_set_size(tile, 130, 82);
  lv_obj_set_pos(tile, x, y);

  // แต่งพื้นหลัง Tile
  lv_obj_set_style_radius(tile, 14, 0);
  lv_obj_set_style_bg_color(tile, lv_color_hex(0x111827), 0);
  lv_obj_set_style_border_width(tile, 1, 0);
  lv_obj_set_style_border_color(tile, lv_color_hex(0x22D3EE), 0);
  lv_obj_set_style_shadow_width(tile, 10, 0);
  lv_obj_set_style_shadow_color(tile, lv_color_hex(0x0EA5E9), 0);

  // ชื่อ Tile
  lv_obj_t *label_title = lv_label_create(tile);
  lv_label_set_text(label_title, title);
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x67E8F9), 0);
  lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 8, 8);

  // คำอธิบาย Tile
  lv_obj_t *label_desc = lv_label_create(tile);
  lv_label_set_text(label_desc, desc);
  lv_obj_set_style_text_color(label_desc, lv_color_hex(0xCBD5E1), 0);
  lv_obj_align(label_desc, LV_ALIGN_TOP_LEFT, 8, 38);

  return tile;
}

/* =========================================================
   สร้างหน้า Desktop หลัง Boot เสร็จ
   ========================================================= */
void create_desktop_screen() {
  // สร้าง screen ใหม่ ไม่ใช้ screen เดิมของ boot
  lv_obj_t *desktop = lv_obj_create(NULL);

  // ตั้งพื้นหลัง
  lv_obj_set_style_bg_color(desktop, lv_color_hex(0x050814), 0);
  lv_obj_set_style_bg_opa(desktop, LV_OPA_COVER, 0);

  // แถบด้านบน
  lv_obj_t *topbar = lv_obj_create(desktop);
  lv_obj_set_size(topbar, 480, 48);
  lv_obj_set_pos(topbar, 0, 0);
  lv_obj_set_style_radius(topbar, 0, 0);
  lv_obj_set_style_bg_color(topbar, lv_color_hex(0x0F172A), 0);
  lv_obj_set_style_border_width(topbar, 0, 0);

  // ชื่อระบบบนแถบบน
  lv_obj_t *title = lv_label_create(topbar);
  lv_label_set_text(title, "TeYoMaRu OS");
  lv_obj_set_style_text_color(title, lv_color_hex(0x67E8F9), 0);
  lv_obj_align(title, LV_ALIGN_LEFT_MID, 16, 0);

  // สถานะขวาบน
  lv_obj_t *status = lv_label_create(topbar);
  lv_label_set_text(status, "READY");
  lv_obj_set_style_text_color(status, lv_color_hex(0x22C55E), 0);
  lv_obj_align(status, LV_ALIGN_RIGHT_MID, -16, 0);

  // การ์ดกลาง
  lv_obj_t *card = lv_obj_create(desktop);
  lv_obj_set_size(card, 448, 245);
  lv_obj_set_pos(card, 16, 62);
  lv_obj_set_style_radius(card, 18, 0);
  lv_obj_set_style_bg_color(card, lv_color_hex(0x0B1120), 0);
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_border_color(card, lv_color_hex(0x1E40AF), 0);

  // ข้อความต้อนรับ
  lv_obj_t *welcome = lv_label_create(card);
  lv_label_set_text(welcome, "Welcome back");
  lv_obj_set_style_text_color(welcome, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(welcome, LV_ALIGN_TOP_LEFT, 18, 16);

  // คำอธิบายเล็ก
  lv_obj_t *sub = lv_label_create(card);
  lv_label_set_text(sub, "Choose a module to continue");
  lv_obj_set_style_text_color(sub, lv_color_hex(0x94A3B8), 0);
  lv_obj_align(sub, LV_ALIGN_TOP_LEFT, 18, 42);

  // Tile 3 อัน
  create_tile(card, 18, 88,  "SCAN", "Device scan");
  create_tile(card, 158, 88, "WIFI", "Network");
  create_tile(card, 298, 88, "SYS",  "System info");

  // ข้อความล่าง
  lv_obj_t *footer = lv_label_create(card);
  lv_label_set_text(footer, "ESP32 + ILI9488 + LVGL");
  lv_obj_set_style_text_color(footer, lv_color_hex(0x64748B), 0);
  lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -12);

  // เปลี่ยนจากหน้า boot มาหน้า desktop
  lv_scr_load_anim(desktop, LV_SCR_LOAD_ANIM_FADE_ON, 400, 0, true);
}

/* =========================================================
   สร้างหน้า Boot Screen
   ========================================================= */
void create_boot_screen() {
  lv_obj_t *screen = lv_scr_act();

  lv_obj_set_style_bg_color(screen, lv_color_hex(0x050814), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  // กล่องกลางจอ
  lv_obj_t *panel = lv_obj_create(screen);
  lv_obj_set_size(panel, 400, 220);
  lv_obj_center(panel);

  lv_obj_set_style_radius(panel, 18, 0);
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x10172A), 0);
  lv_obj_set_style_border_color(panel, lv_color_hex(0x22D3EE), 0);
  lv_obj_set_style_border_width(panel, 2, 0);
  lv_obj_set_style_shadow_width(panel, 24, 0);
  lv_obj_set_style_shadow_color(panel, lv_color_hex(0x0EA5E9), 0);

  // ชื่อระบบ
  lv_obj_t *title = lv_label_create(panel);
  lv_label_set_text(title, "TeYoMaRu OS");
  lv_obj_set_style_text_color(title, lv_color_hex(0x67E8F9), 0);

  /*
     ใช้ LV_FONT_DEFAULT แทน lv_font_montserrat_28
     เพราะบางโปรเจกต์ไม่ได้เปิด font 28 ไว้
     จะช่วยลด error ตอน compile
  */
  lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

  // ข้อความสถานะ
  status_label = lv_label_create(panel);
  lv_label_set_text(status_label, "Starting display engine...");
  lv_obj_set_style_text_color(status_label, lv_color_hex(0xE5E7EB), 0);
  lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 78);

  // Progress bar
  boot_bar = lv_bar_create(panel);
  lv_obj_set_size(boot_bar, 300, 18);
  lv_obj_align(boot_bar, LV_ALIGN_TOP_MID, 0, 128);
  lv_bar_set_range(boot_bar, 0, 100);
  lv_bar_set_value(boot_bar, 0, LV_ANIM_OFF);

  lv_obj_set_style_radius(boot_bar, 9, LV_PART_MAIN);
  lv_obj_set_style_radius(boot_bar, 9, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(boot_bar, lv_color_hex(0x1F2937), LV_PART_MAIN);
  lv_obj_set_style_bg_color(boot_bar, lv_color_hex(0x22D3EE), LV_PART_INDICATOR);

  // ตัวเลข %
  percent_label = lv_label_create(panel);
  lv_label_set_text(percent_label, "0%");
  lv_obj_set_style_text_color(percent_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(percent_label, LV_ALIGN_TOP_MID, 0, 158);
}

/* =========================================================
   Timer ของหน้า Boot
   เพิ่มเปอร์เซ็นต์และเปลี่ยนข้อความตามช่วง
   ========================================================= */
void boot_timer_cb(lv_timer_t *timer) {
  boot_percent += 2;

  if (boot_percent > 100) {
    boot_percent = 100;
  }

  lv_bar_set_value(boot_bar, boot_percent, LV_ANIM_ON);

  static char buf[16];
  snprintf(buf, sizeof(buf), "%d%%", boot_percent);
  lv_label_set_text(percent_label, buf);

  if (boot_percent == 25) {
    lv_label_set_text(status_label, "Loading TeYoMaRu core...");
  } else if (boot_percent == 55) {
    lv_label_set_text(status_label, "Preparing desktop...");
  } else if (boot_percent == 85) {
    lv_label_set_text(status_label, "Almost ready...");
  } else if (boot_percent >= 100) {
    lv_label_set_text(status_label, "Boot complete");

    // ลบ timer ไม่ให้ทำงานซ้ำ
    lv_timer_del(timer);
    boot_timer = NULL;

    // เมื่อ boot เสร็จ ให้เข้า Desktop
    create_desktop_screen();
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("TeYoMaRu OS LVGL Starter");

  // เริ่มจอ TFT
  tft.init();
  tft.invertDisplay(true);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // เริ่ม LVGL
  lv_init();

  // ตั้ง buffer ให้ LVGL
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_WIDTH * 40);

  // ตั้ง display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  disp_drv.hor_res = SCREEN_WIDTH;
  disp_drv.ver_res = SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;

  lv_disp_drv_register(&disp_drv);

  // สร้างหน้า Boot
  create_boot_screen();

  // ให้ boot timer ทำงานทุก 80 ms
  boot_timer = lv_timer_create(boot_timer_cb, 80, NULL);
}

void loop() {
  // ให้ LVGL ทำงานตลอด
  lv_timer_handler();

  // หน่วงเวลาเล็กน้อย
  delay(5);

  // บอก LVGL ว่าเวลาผ่านไป 5 ms
  lv_tick_inc(5);
}