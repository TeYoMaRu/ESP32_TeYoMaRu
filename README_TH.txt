TeYoMaRu OS - LVGL Starter
==========================

ชุดนี้ทำไว้สำหรับ:
- ESP32 DevKit V1 Type-C
- ESP32-WROOM-32
- จอ TFT SPI 4.0 นิ้ว 480x320
- Driver: ILI9488
- ใช้ PlatformIO + Arduino Framework
- ใช้ TFT_eSPI + LVGL 8.x

----------------------------------
1) การต่อสาย ESP32 -> TFT ILI9488
----------------------------------

TFT Pin        ESP32 Pin
------------------------
VCC            3V3
GND            GND
SDI(MOSI)      D23 / GPIO23
SDO(MISO)      D19 / GPIO19
SCK            D18 / GPIO18
CS             D5  / GPIO5
DC / RS        D2  / GPIO2
RESET          D4  / GPIO4
LED            3V3

หมายเหตุ:
- ตอนนี้ยังไม่ต่อ Touch
- ตอนนี้ยังไม่ต่อ SD Card
- เริ่มจากจออย่างเดียวก่อน เพื่อลดจุดผิดพลาด

----------------------------------
2) ค่าที่แก้ใน platformio.ini
----------------------------------

Library:
- bodmer/TFT_eSPI@^2.5.43
- lvgl/lvgl@^8.3.11

TFT_eSPI build_flags:
- USER_SETUP_LOADED
- ILI9488_DRIVER
- TFT_WIDTH=320
- TFT_HEIGHT=480
- TFT_MOSI=23
- TFT_MISO=19
- TFT_SCLK=18
- TFT_CS=5
- TFT_DC=2
- TFT_RST=4
- SPI_FREQUENCY=40000000
- SPI_READ_FREQUENCY=16000000

Font flags ที่ต้องเปิด ไม่อย่างนั้นตัวหนังสือไม่ขึ้น:
- LOAD_GLCD
- LOAD_FONT2
- LOAD_FONT4
- LOAD_FONT6
- LOAD_FONT7
- LOAD_FONT8
- SMOOTH_FONT

LVGL flags:
- LV_CONF_INCLUDE_SIMPLE
- -I include

----------------------------------
3) ค่าที่สำคัญใน main.cpp
----------------------------------

จอของคุณต้องใช้:

tft.invertDisplay(true);

เพราะถ้าไม่ใส่:
- สีดำอาจกลายเป็นสีขาว
- สีอาจเพี้ยน

จอใช้แนวนอน:

tft.setRotation(1);

ขนาดหลังหมุน:
- กว้าง 480
- สูง 320

----------------------------------
4) ค่าที่สำคัญใน LVGL
----------------------------------

ไฟล์ include/lv_conf.h ตั้งไว้แบบนี้:
- LV_COLOR_DEPTH 16
- LV_MEM_SIZE 48 KB
- เปิด LABEL, BAR, BTN, IMG, LINE
- เปิด Theme Default แบบ Dark

Buffer ใน main.cpp:
- SCREEN_WIDTH * 40
- เท่ากับ 480 x 40 pixel

เหตุผล:
- ถ้า buffer ใหญ่เกินไป ESP32 อาจ RAM ไม่พอ
- 480 x 40 เหมาะสำหรับเริ่มต้นกับ ESP32

----------------------------------
5) วิธีใช้งาน
----------------------------------

1. แตก ZIP
2. เปิดโฟลเดอร์ TeYoMaRu_LVGL_Starter ด้วย VS Code
3. รอ PlatformIO โหลด Library
4. กด Build
5. ถ้า Build ผ่าน กด Upload
6. จอควรขึ้นหน้า Boot Screen ของ TeYoMaRu OS

----------------------------------
6) ถ้าเจอปัญหา
----------------------------------

จอขาว:
- เช็กว่า main.cpp มี tft.invertDisplay(true);
- เช็ก LED ต่อ 3V3
- เช็ก GND

ตัวหนังสือไม่ขึ้น:
- เช็ก platformio.ini ว่ามี LOAD_GLCD และ LOAD_FONT ต่าง ๆ
- กด Clean ก่อน Upload ใหม่

Upload ไม่ผ่าน:
- กดปุ่ม BOOT ที่บอร์ด ESP32 ค้างไว้ตอนเริ่ม Upload
- ปล่อยปุ่มเมื่อขึ้น Writing...

ภาพกลับหัว:
- เปลี่ยน tft.setRotation(1); เป็น tft.setRotation(3);

----------------------------------
7) ขั้นต่อไป
----------------------------------

เมื่อ Boot Screen นี้ขึ้นแล้ว ขั้นต่อไปจะทำ:
- แยกไฟล์ display.cpp / display.h
- แยกไฟล์ boot.cpp / boot.h
- เพิ่มหน้า Desktop
- เพิ่ม Touch ด้วย XPT2046
- เริ่มสร้าง TeYoMaRu OS จริง
