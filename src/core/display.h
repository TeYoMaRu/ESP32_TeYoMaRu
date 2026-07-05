#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>

// ขนาดจอหลังหมุนแนวนอน
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320

// ขา Backlight ของจอ
#define TFT_BL 15

// ฟังก์ชันเริ่มต้นระบบจอทั้งหมด
void display_init();

// เปิดไฟ Backlight
void display_backlight_on();

// ปิดไฟ Backlight
void display_backlight_off();

#endif