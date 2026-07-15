#include "touch.h"
#include "display.h"
#include <lvgl.h>
#include <XPT2046_Touchscreen.h>
#include <Arduino.h>

#define TOUCH_CS   22
#define TOUCH_IRQ  21   // ← ต้องต่อสาย T_IRQ → GPIO21

// Calibration: ปรับตามค่า raw จริงที่กดมุมจอ
// จาก log ที่เห็น x raw ~508..3520, y raw ~12..507
// แต่จอหมุน 90° ดังนั้น x↔y สลับกัน
#define TOUCH_X_MIN   400
#define TOUCH_X_MAX  3600
#define TOUCH_Y_MIN   200
#define TOUCH_Y_MAX  3800

// กดซ้ำต้องขยับ > 10px จึงถือว่า event ใหม่
#define MOVE_THRESHOLD 10

static XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

static int16_t _last_x = -1, _last_y = -1;

static void my_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  // IRQ mode: ts.tirqTouched() เร็วกว่า ts.touched() มาก
  if (!ts.tirqTouched() || !ts.touched()) {
    data->state = LV_INDEV_STATE_RELEASED;
    _last_x = _last_y = -1;
    return;
  }

  TS_Point p = ts.getPoint();

  // กรอง noise: z ต้องอยู่ในช่วงที่สมเหตุสมผล
  if (p.z < 200 || p.z > 4090) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  // จอ rotation=1 (landscape) → x และ y ของ touch สลับกัน
  int16_t x = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, SCREEN_WIDTH  - 1);
  int16_t y = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, SCREEN_HEIGHT - 1);
  x = constrain(x, 0, SCREEN_WIDTH  - 1);
  y = constrain(y, 0, SCREEN_HEIGHT - 1);

  Serial.printf("[TOUCH] x=%d y=%d (raw %d,%d z=%d)\n", x, y, p.x, p.y, p.z);

  data->point.x = x;
  data->point.y = y;
  data->state   = LV_INDEV_STATE_PRESSED;
}

void touch_init() {
  ts.begin();
  ts.setRotation(1);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type    = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touch_read;
  lv_indev_drv_register(&indev_drv);

  Serial.println("[TOUCH] ready (IRQ mode GPIO21)");
}
