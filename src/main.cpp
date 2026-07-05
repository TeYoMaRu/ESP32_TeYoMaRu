#include <Arduino.h>
#include "core/display.h"
#include "pages/boot.h"

#define TFT_BL 15

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("TeYoMaRu OS Start");


  display_init();
  boot_create();
  boot_start();
}

void loop() {
  lv_timer_handler();
  delay(5);
  lv_tick_inc(5);
}