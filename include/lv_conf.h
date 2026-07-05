#ifndef LV_CONF_H
#define LV_CONF_H

/* =========================================================
   LVGL Configuration for TeYoMaRu OS Starter
   Board  : ESP32 DevKit V1
   Display: ILI9488 SPI 480x320
   LVGL   : 8.x
   ========================================================= */

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (48U * 1024U)

#define LV_TICK_CUSTOM 0
#define LV_USE_LOG 0

#define LV_HOR_RES_MAX 480
#define LV_VER_RES_MAX 320

#define LV_DPI_DEF 130

/* Widgets ที่ใช้ในตัวอย่าง Boot Screen */
#define LV_USE_LABEL 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_IMG 1
#define LV_USE_LINE 1

#define LV_FONT_MONTSERRAT_8  1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1

/* Theme */
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 1
#define LV_THEME_DEFAULT_GROW 1
#define LV_THEME_DEFAULT_TRANSITION_TIME 80

#endif
