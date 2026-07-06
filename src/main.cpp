#include <Arduino.h>
#include "core/ui.h"
#include "pages/boot.h"

/* ======================================================
   TeYoMaRu OS v0.2 Yellow Edition
   ESP32 + TFT_eSPI + LVGL

   วิธีทำงาน:
   1. ui_init()      เตรียมจอ + LVGL
   2. boot_create()  สร้างหน้า Boot
   3. boot_start()   เริ่มโหลด Progress
   4. ครบ 100% เข้า Desktop แบบไม่ Fade เพื่อลดกระตุก
====================================================== */

void setup() {
  ui_init();
  boot_create();
  boot_start();
}

void loop() {
  ui_loop();
}
