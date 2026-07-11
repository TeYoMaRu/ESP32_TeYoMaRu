#include "ui.h"
#include "display.h"
#include <Arduino.h>
#include <lvgl.h>

/* ======================================================
   UI Bootstrap
   ไฟล์นี้เป็นจุดเริ่มต้นของระบบ UI ทั้งหมด
   - เรียก display_init() จาก display.cpp เพื่อตั้งค่าจอ TFT + LVGL
     ให้ตรงกับฮาร์ดแวร์จริง (480x320, invertDisplay, buffer ที่ถูกต้อง)
   - ทำหน้าที่ป้อน "tick" ให้ LVGL รู้ว่าเวลาผ่านไปเท่าไหร่
     (จำเป็นเพราะ lv_conf.h ตั้ง LV_TICK_CUSTOM เป็น 0
      แปลว่า LVGL จะไม่นับเวลาเองถ้าเราไม่ป้อนให้)
====================================================== */

void ui_init() {
  Serial.begin(115200);
  display_init();   // ตั้งค่าจอ + LVGL ทั้งหมดอยู่ใน display.cpp
}

void ui_loop() {
  static uint32_t last_tick = 0;
  uint32_t now = millis();

  // ป้อนเวลาที่ผ่านไปให้ LVGL ทุกรอบ ไม่งั้น timer/animation จะไม่ขยับ
  lv_tick_inc(now - last_tick);
  last_tick = now;

  lv_timer_handler();
  delay(5);
}
