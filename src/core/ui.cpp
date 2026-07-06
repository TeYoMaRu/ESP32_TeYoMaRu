#include "ui.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

/* ======================================================
   LVGL + TFT_eSPI Bridge
   ไฟล์นี้ทำหน้าที่เชื่อม LVGL เข้ากับจอ TFT
   ถ้า pin จอไม่ตรง ให้แก้ที่ platformio.ini ตรง build_flags
====================================================== */

static TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[240 * 20];  // buffer เล็กเพื่อลด RAM

static void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void ui_init() {
  Serial.begin(115200);

  pinMode(15, OUTPUT);       // Backlight GPIO15
  digitalWrite(15, HIGH);    // เปิดไฟจอ

  lv_init();
  tft.begin();
  tft.setRotation(1);        // แนวนอน 320x240
  tft.fillScreen(TFT_BLACK);

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, 240 * 20);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = display_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

void ui_loop() {
  lv_timer_handler();
  delay(5);
}
