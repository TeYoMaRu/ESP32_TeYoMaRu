#pragma once

#include <Arduino.h>
#include <lvgl.h>

// ขนาดจอเมื่อวางแนวนอน
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320

// ปรับให้ตรงกับขา Backlight ของโปรเจกต์เดิม
#ifndef TFT_BL
#define TFT_BL 15
#endif

void display_init();
void display_backlight_on();
void display_backlight_off();

// ใช้หมุนเฉพาะหน้า QR
void display_set_landscape();
void display_set_portrait();
bool display_is_portrait();
