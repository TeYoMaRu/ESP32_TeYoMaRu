#include "display.h"
#include <TFT_eSPI.h>

static TFT_eSPI tft = TFT_eSPI();

// LVGL draw buffer  (2 แถว × 480px × 2 bytes ต่อ pixel × 10)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_WIDTH * 10];

/*
  ILI9488 ต้องการข้อมูล 18-bit (RGB666) แต่ LVGL ส่ง RGB565
  pushColors ของ TFT_eSPI จะ convert ให้อัตโนมัติเมื่อใช้ pushPixels
  แต่ถ้าใช้ pushColors((uint16_t*)…) กับ ILI9488 จะสีผิด
  วิธีแก้: ใช้ tft.pushPixelsDMA หรือ loop writeColor ทีละ pixel
  ─── แก้ flush ให้แปลง RGB565 → RGB888 ก่อนส่ง ───
*/
static void my_disp_flush(lv_disp_drv_t *disp,
                           const lv_area_t *area,
                           lv_color_t *color_p) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;
  uint32_t n = w * h;

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);

  // ส่ง pixel ทีละตัว: TFT_eSPI จะแปลง 16-bit → 18-bit ให้
  // (writeColor ใช้ SPI transaction เดียว ไม่ช้ามาก)
  for (uint32_t i = 0; i < n; i++) {
    tft.writeColor(color_p->full, 1);
    color_p++;
  }

  tft.endWrite();
  lv_disp_flush_ready(disp);
}

void display_init() {
  display_backlight_on();

  tft.begin();           // init SPI + IC
  tft.setRotation(1);    // 0=Portrait, 1=Landscape 480×320
  tft.invertDisplay(false);
  tft.fillScreen(TFT_BLACK);

  lv_init();

  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_WIDTH * 10);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = SCREEN_WIDTH;   // 480
  disp_drv.ver_res  = SCREEN_HEIGHT;  // 320
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

void display_backlight_on() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
}

void display_backlight_off() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
}
