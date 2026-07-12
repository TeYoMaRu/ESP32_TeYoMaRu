#include "ui.h"
#include "display.h"
#include "touch.h"
#include <Arduino.h>
#include <lvgl.h>

void ui_init() {
  Serial.begin(115200);
  display_init();   // TFT + LVGL display driver
  touch_init();     // XPT2046 + LVGL input driver
}

void ui_loop() {
  static uint32_t last_tick = 0;
  uint32_t now = millis();
  lv_tick_inc(now - last_tick);
  last_tick = now;
  lv_timer_handler();
  delay(5);
}
