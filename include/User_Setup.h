/* =========================================================
   User_Setup.h  –  ESP32 + ILI9488 4.0" 480x320
   ========================================================= */
#ifndef USER_SETUP_H
#define USER_SETUP_H

#define ILI9488_DRIVER

#define TFT_WIDTH  320
#define TFT_HEIGHT 480

#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_SCLK 18
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
#define TFT_BL   15

// Touch CS บอก TFT_eSPI ว่ามี touch บน bus นี้
// (XPT2046_Touchscreen จัดการเองแยกต่างหาก)
#define TOUCH_CS 22

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY       20000000
#define SPI_READ_FREQUENCY  16000000
#define SPI_TOUCH_FREQUENCY  2500000

#endif
