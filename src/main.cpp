#include <Arduino.h>

#include "core/ui.h"
#include "pages/boot.h"

/* =========================================================
   TeYoMaRu OS - main.cpp

   หน้าที่ของไฟล์นี้
   1. เปิด Serial Monitor
   2. เริ่มระบบจอ TFT + LVGL + Touch ผ่าน ui_init()
   3. สร้างหน้า Boot
   4. เรียก ui_loop() ตลอดเวลาเพื่อให้จอและทัชทำงาน

   หมายเหตุ:
   - ค่าขาทัชและค่า Calibration ไม่ได้อยู่ในไฟล์นี้
   - ค่าดังกล่าวอยู่ที่ src/core/touch.cpp
   ========================================================= */

void setup() {
  // เปิด Serial Monitor
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("================================");
  Serial.println(" TeYoMaRu OS starting...");
  Serial.println("================================");

  // เริ่มระบบจอ TFT, LVGL และ Touch
  ui_init();

  // สร้างและเริ่มหน้า Boot
  boot_create();
  boot_start();

  Serial.println("[SYSTEM] ready");
}

void loop() {
  // ประมวลผล LVGL และการสัมผัสหน้าจอ
  ui_loop();
}
