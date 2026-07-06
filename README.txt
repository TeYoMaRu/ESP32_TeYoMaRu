TeYoMaRu OS v0.2 Yellow Edition
================================

ธีมสี: เหลือง / ขาว / ส้ม / แดง
เหตุผล: จอ TFT ที่สว่างไม่มากจะเห็นสีเหลืองและขาวชัดกว่าสีฟ้า

วิธีใช้:
1. แตก zip
2. เปิดโฟลเดอร์ด้วย VS Code + PlatformIO
3. ต่อ ESP32
4. กด Build
5. กด Upload

ถ้าจอไม่ติดหรือสีเพี้ยน:
- แก้ pin ใน platformio.ini
- เช็ก TFT_MOSI, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST, TFT_BL
- ตอนนี้ตั้ง Backlight เป็น GPIO15

ถ้าเข้าหน้า Desktop แล้วยังกระตุก:
- ชุดนี้ไม่ใช้ Fade แล้ว
- ลด animation และใช้ lv_scr_load() ตรง ๆ
- ใช้ LVGL buffer เล็ก 240*20 เพื่อลด RAM

ไฟล์สำคัญ:
- src/main.cpp
- src/core/ui.cpp
- src/core/theme.h
- src/pages/boot.cpp
- src/pages/desktop.cpp
- src/pages/app_page.cpp
- src/widgets/tile.cpp
- src/widgets/statusbar.cpp
