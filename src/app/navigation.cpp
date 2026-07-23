#include "navigation.h"

#include <cstring>

#include "../pages/desktop.h"
#include "../pages/app_page.h"
#include "../pages/scan_page.h"
#include "../pages/wifi_page.h"
#include "../pages/bluetooth_page.h"
#include "../pages/settings_page.h"

/* ======================================================
   เปิดหน้า Desktop
====================================================== */

void nav_open_desktop() {
  desktop_create();
}

/* ======================================================
   เปิดหน้า Scan
====================================================== */

void nav_open_scan() {
  scan_page_create();
}

/* ======================================================
   เปิดหน้า WiFi
====================================================== */

void nav_open_wifi() {
  wifi_page_create();
}

/* ======================================================
   เปิดหน้า Bluetooth
====================================================== */

void nav_open_bluetooth() {
  bluetooth_page_create();
}


void nav_open_settings() {
    settings_page_create();
}
/* ======================================================
   เปิดแอปจากชื่อ Tile

   Routing ทั้งหมดรวมอยู่ตรงนี้
   Desktop ไม่ต้องรู้ว่าแต่ละหน้าใช้ฟังก์ชันอะไร
====================================================== */

void nav_open_app(const char *title) {
  if (!title || title[0] == '\0') {
    return;
  }

  if (strcmp(title, "Scan") == 0) {
    nav_open_scan();
    return;
  }

  if (strcmp(title, "WiFi") == 0) {
    nav_open_wifi();
    return;
  }

  if (strcmp(title, "Bluetooth") == 0) {
    nav_open_bluetooth();
    return;
  }

  if (strcmp(title, "Settings") == 0) {
    nav_open_settings();
    return;
}

  /*
    แอปที่ยังไม่มีหน้าจริง
    ใช้หน้า Coming Soon จาก app_page.cpp
  */
  app_page_create(title);
}