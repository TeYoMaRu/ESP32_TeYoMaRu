#ifndef THEME_H
#define THEME_H

#include <lvgl.h>

/* ======================================================
   TeYoMaRu OS - Theme System

   ไฟล์นี้รวมสี + ฟังก์ชันแต่งหน้าจอที่ไฟล์อื่นเรียกใช้
   สำคัญ: ชุดโค้ดปัจจุบันยังมีทั้งชื่อเก่า COLOR_* และชื่อใหม่ C_*
   จึงประกาศ alias ไว้ให้ทั้งสองแบบ เพื่อไม่ให้ build พัง
====================================================== */

#define C_BG        lv_color_hex(0x050814)
#define C_PANEL     lv_color_hex(0x10172A)
#define C_CARD      lv_color_hex(0x0B1120)
#define C_BLUE      lv_color_hex(0xFFD43B)
#define C_BLUE_DARK lv_color_hex(0xFFB000)
#define C_TEXT      lv_color_hex(0xFFFFFF)
#define C_MUTED     lv_color_hex(0xAAB4C8)
#define C_GREEN     lv_color_hex(0x22C55E)
#define C_RED       lv_color_hex(0xFF3B30)
#define C_ORANGE    lv_color_hex(0xFFB000)
#define C_BORDER    lv_color_hex(0x26324A)
#define C_PANEL_2   lv_color_hex(0x111827)

// ชื่อเก่าที่ไฟล์ boot/tile/statusbar/popup/app_page ยังเรียกใช้อยู่
#define COLOR_BG        C_BG
#define COLOR_PANEL     C_PANEL
#define COLOR_PANEL_2   C_PANEL_2
#define COLOR_PRIMARY   C_BLUE
#define COLOR_TEXT      C_TEXT
#define COLOR_MUTED     C_MUTED
#define COLOR_GREEN     C_GREEN
#define COLOR_ORANGE    C_ORANGE
#define COLOR_BORDER    C_BORDER

void theme_apply_screen(lv_obj_t *obj);
void theme_apply_panel(lv_obj_t *obj);
void theme_apply_label(lv_obj_t *obj, bool big);
void theme_apply_small_label(lv_obj_t *obj, bool center);

#endif
