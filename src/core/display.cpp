#include "display.h"
#include <TFT_eSPI.h>

static TFT_eSPI tft = TFT_eSPI();

// Buffer 10 แถว รองรับทั้ง 480x320 และ 320x480
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_WIDTH * 10];
static lv_disp_drv_t disp_drv;
static bool disp_registered = false;
static bool portrait_mode = false;

static void my_disp_flush(
  lv_disp_drv_t *disp,
  const lv_area_t *area,
  lv_color_t *color_p
) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;
  uint32_t n = w * h;

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);

  for (uint32_t i = 0; i < n; i++) {
    tft.writeColor(color_p->full, 1);
    color_p++;
  }

  tft.endWrite();
  lv_disp_flush_ready(disp);
}

static void apply_orientation(bool portrait) {
  portrait_mode = portrait;

  if (portrait_mode) {
    tft.setRotation(0);       // 320x480
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 480;
  } else {
    tft.setRotation(1);       // 480x320
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;
  }

  if (disp_registered) {
    lv_disp_drv_update(lv_disp_get_default(), &disp_drv);
    lv_obj_invalidate(lv_scr_act());
  }

  tft.fillScreen(TFT_BLACK);
}

void display_init() {
  display_backlight_on();

  tft.begin();
  tft.setRotation(1);
  tft.invertDisplay(false);
  tft.fillScreen(TFT_BLACK);

  lv_init();

  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_WIDTH * 10);

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = SCREEN_WIDTH;
  disp_drv.ver_res  = SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;

  lv_disp_drv_register(&disp_drv);
  disp_registered = true;
  portrait_mode = false;
}

void display_set_landscape() {
  apply_orientation(false);
}

void display_set_portrait() {
  apply_orientation(true);
}

bool display_is_portrait() {
  return portrait_mode;
}

void display_backlight_on() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
}

void display_backlight_off() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
}
