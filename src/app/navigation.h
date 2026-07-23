#pragma once

/*
  Navigation กลางของ TeYoMaRu OS

  ทุกหน้าควรเปิดผ่านฟังก์ชันในไฟล์นี้
  ไม่ควรเรียก page_create() ข้ามหน้ากันโดยตรง
*/

void nav_open_desktop();
void nav_open_scan();
void nav_open_wifi();
void nav_open_bluetooth();
void nav_open_settings();

/*
  ใช้เปิดแอปจากชื่อ Tile

  แอปที่มีหน้าจริง:
  - Scan
  - WiFi
  - Bluetooth

  แอปที่ยังไม่มีหน้าจริง:
  - เปิดหน้า Coming Soon
*/
void nav_open_app(const char *title);