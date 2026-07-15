#include "touch.h"
#include "display.h"

#include <Arduino.h>
#include <lvgl.h>
#include <XPT2046_Touchscreen.h>

/* =========================================================
   TeYoMaRu OS - XPT2046 Touch Driver

   การต่อสายจริง
   ---------------------------------------------------------
   T_CS   -> GPIO22
   T_IRQ  -> GPIO21
   T_CLK  -> GPIO18
   T_DIN  -> GPIO23
   T_DO   -> GPIO19

   ผลการทดสอบ 4 มุมของจอจริง
   ---------------------------------------------------------
   ซ้ายบน   : raw X ≈ 3510, raw Y ≈ 280
   ขวาบน    : raw X ≈ 3550, raw Y ≈ 3650
   ซ้ายล่าง : raw X ≈ 550,  raw Y ≈ 315
   ขวาล่าง  : raw X ≈ 500,  raw Y ≈ 3700

   สรุป:
   - แกนหน้าจอ X ต้องอ่านจาก raw Y
   - แกนหน้าจอ Y ต้องอ่านจาก raw X
   - แกน Y ต้องกลับทิศ
   ========================================================= */

// ขาควบคุมระบบ Touch
#define TOUCH_CS   22
#define TOUCH_IRQ  21

// ค่าคาลิเบรตจากจอจริง
#define TOUCH_RAW_X_TOP     3535
#define TOUCH_RAW_X_BOTTOM   500

#define TOUCH_RAW_Y_LEFT     290
#define TOUCH_RAW_Y_RIGHT   3700

// ค่าความแรงกดขั้นต่ำ
#define TOUCH_MIN_PRESSURE 200

static XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

/* =========================================================
   อ่านตำแหน่ง Touch ให้ LVGL
   ========================================================= */
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

  /*
    จอแนวนอน 480 × 320

    screen_x ใช้ raw Y:
      ซ้าย  ≈ 290
      ขวา   ≈ 3700

    screen_y ใช้ raw X และกลับทิศ:
      บน    ≈ 3535
      ล่าง  ≈ 500
  */
  int16_t screen_x = map(
    point.y,
    TOUCH_RAW_Y_LEFT,
    TOUCH_RAW_Y_RIGHT,
    0,
    SCREEN_WIDTH - 1
  );

  int16_t screen_y = map(
    point.x,
    TOUCH_RAW_X_TOP,
    TOUCH_RAW_X_BOTTOM,
    0,
    SCREEN_HEIGHT - 1
  );

  screen_x = constrain(screen_x, 0, SCREEN_WIDTH - 1);
  screen_y = constrain(screen_y, 0, SCREEN_HEIGHT - 1);

  data->point.x = screen_x;
  data->point.y = screen_y;
  data->state   = LV_INDEV_STATE_PRESSED;

  Serial.printf(
    "[TOUCH] x=%d y=%d (raw %d,%d z=%d)\n",
    screen_x,
    screen_y,
    point.x,
    point.y,
    point.z
  );
}

/* =========================================================
   เริ่มระบบ Touch
   ========================================================= */
void touch_init() {
  touch.begin();

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);

  indev_drv.type    = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touch_read_cb;

  lv_indev_drv_register(&indev_drv);

  Serial.println("[TOUCH] ready  CS=GPIO22  IRQ=GPIO21");
  Serial.println("[TOUCH] axes swapped: screen X=rawY, screen Y=rawX reversed");
}