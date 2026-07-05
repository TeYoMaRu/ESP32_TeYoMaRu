#include "display.h"
#include <TFT_eSPI.h>

// สร้าง object จอ TFT
static TFT_eSPI tft = TFT_eSPI();

// Buffer ของ LVGL
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_WIDTH * 40];

/*
   ฟังก์ชันส่งภาพจาก LVGL ไปยังจอ TFT จริง
*/
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/*
   เริ่มต้น TFT + LVGL
*/
void display_init() {
    display_backlight_on();
    
  tft.init();
  tft.invertDisplay(true);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  lv_init();

  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_WIDTH * 40);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  disp_drv.hor_res = SCREEN_WIDTH;
  disp_drv.ver_res = SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;

  lv_disp_drv_register(&disp_drv);
}

/*
   เปิดไฟ Backlight ของจอ

   GPIO15 ต่อกับ LED / Backlight ของจอ
   ถ้าไม่เปิดขานี้ จอจะติดแต่ภาพจะมืดมาก
*/
void display_backlight_on() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
}

/*
   ปิดไฟ Backlight ของจอ

   ใช้ในอนาคตได้ เช่น
   - Sleep mode
   - ประหยัดพลังงาน
   - ปิดจอชั่วคราว
*/
void display_backlight_off() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
}