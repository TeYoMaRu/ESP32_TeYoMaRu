#include "touch.h"
#include "display.h"

#include <Arduino.h>
#include <lvgl.h>
#include <XPT2046_Touchscreen.h>

#define TOUCH_CS   22
#define TOUCH_IRQ  21

#define TOUCH_RAW_X_TOP      3535
#define TOUCH_RAW_X_BOTTOM    500
#define TOUCH_RAW_Y_LEFT      290
#define TOUCH_RAW_Y_RIGHT    3700

#define TOUCH_MIN_PRESSURE 200

static XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

static void touch_read_cb(
  lv_indev_drv_t *indev_drv,
  lv_indev_data_t *data
) {
  (void)indev_drv;

  if (!touch.touched()) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  TS_Point point = touch.getPoint();

  if (point.z < TOUCH_MIN_PRESSURE || point.z >= 4095) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  // คำนวณตำแหน่งพื้นฐานแบบแนวนอนก่อน
  int16_t landscape_x = map(
    point.y,
    TOUCH_RAW_Y_LEFT,
    TOUCH_RAW_Y_RIGHT,
    0,
    479
  );

  int16_t landscape_y = map(
    point.x,
    TOUCH_RAW_X_TOP,
    TOUCH_RAW_X_BOTTOM,
    0,
    319
  );

  landscape_x = constrain(landscape_x, 0, 479);
  landscape_y = constrain(landscape_y, 0, 319);

  int16_t screen_x;
  int16_t screen_y;

  if (display_is_portrait()) {
    /*
      TFT_eSPI ของจอชุดนี้หมุนจาก Rotation 1 ไป Rotation 0
      ในทิศตรงข้ามกับสูตรเดิม จึงต้องกลับแกน X และใช้
      landscape_x เป็นแกน Y โดยตรง

      Portrait 320x480:
        X = 319 - landscape_y
        Y = landscape_x
    */
    screen_x = 319 - landscape_y;
    screen_y = landscape_x;

    screen_x = constrain(screen_x, 0, 319);
    screen_y = constrain(screen_y, 0, 479);
  } else {
    screen_x = landscape_x;
    screen_y = landscape_y;
  }

  data->point.x = screen_x;
  data->point.y = screen_y;
  data->state   = LV_INDEV_STATE_PRESSED;

  Serial.printf(
    "[TOUCH] mode=%s x=%d y=%d raw=%d,%d z=%d\n",
    display_is_portrait() ? "PORTRAIT" : "LANDSCAPE",
    screen_x,
    screen_y,
    point.x,
    point.y,
    point.z
  );
}

void touch_init() {
  touch.begin();

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);

  indev_drv.type    = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touch_read_cb;

  lv_indev_drv_register(&indev_drv);

  Serial.println("[TOUCH] ready CS=GPIO22 IRQ=GPIO21");
  Serial.println("[TOUCH] supports landscape and portrait");
}
