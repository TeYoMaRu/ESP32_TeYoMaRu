TeYoMaRu OS - Desktop Home Style

ไฟล์ในชุดนี้:
- desktop.cpp
- tile.cpp
- tile.h

ตำแหน่งที่นำไปแทน:
src/pages/desktop.cpp
src/widgets/tile.cpp
src/widgets/tile.h

สิ่งที่แก้:
- จัด Tile 4 คอลัมน์ x 2 แถว
- ชื่อ App อยู่ใต้กล่องไอคอน
- เพิ่มขอบและเงาสีน้ำเงิน
- ใช้ LVGL Symbol แทนตัวอักษรธรรมดา
- เพิ่มจุดบอกหน้าด้านล่าง
- ใส่คอมเมนท์ละเอียดสำหรับปรับตำแหน่งและขนาด

หมายเหตุ:
ต้องเปิด Font Montserrat 14 และ 28 ใน lv_conf.h

LV_FONT_MONTSERRAT_14 1
LV_FONT_MONTSERRAT_28 1

หากยังไม่ได้เปิด 28 ให้เปลี่ยนใน tile.cpp:
&lv_font_montserrat_28
เป็น:
&lv_font_montserrat_24
