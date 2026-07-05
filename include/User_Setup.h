/* =========================================================
   User_Setup.h
   สำหรับ ESP32 DevKit V1 Type-C + TFT SPI 4.0" ILI9488
   ความละเอียด 480x320

   การต่อสายที่ใช้ในชุดนี้
   ---------------------------------------------------------
   TFT VCC        -> ESP32 3V3
   TFT GND        -> ESP32 GND
   TFT CS         -> ESP32 GPIO5  / D5
   TFT RESET      -> ESP32 GPIO4  / D4
   TFT DC/RS      -> ESP32 GPIO2  / D2
   TFT SDI(MOSI)  -> ESP32 GPIO23 / D23
   TFT SCK        -> ESP32 GPIO18 / D18
   TFT LED        -> ESP32 3V3
   TFT SDO(MISO)  -> ESP32 GPIO19 / D19
   ---------------------------------------------------------
   ตอนนี้ยังไม่ตั้ง Touch และ SD Card
   เอาให้จอติดก่อนค่ะ
   ========================================================= */

#ifndef USER_SETUP_H
#define USER_SETUP_H

// เลือก Driver ของจอ
#define ILI9488_DRIVER

// ขนาดจอจริงของรุ่นนี้
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// ขา SPI หลักของ ESP32
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_SCLK 18

// ขาควบคุมจอ
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4

// ใช้ SPI ของ ESP32
#define USE_HSPI_PORT

// ฟอนต์พื้นฐานของ TFT_eSPI
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// ความเร็ว SPI
// ถ้าจอขาวหรือเพี้ยน ให้ลองลดเป็น 27000000
#define SPI_FREQUENCY       27000000
#define SPI_READ_FREQUENCY  16000000
#define SPI_TOUCH_FREQUENCY 2500000

#endif
