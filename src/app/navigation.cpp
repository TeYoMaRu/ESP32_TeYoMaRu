#include <cstring>
#include "navigation.h"

#include "../pages/desktop.h"
#include "../pages/app_page.h"
#include "../pages/scan_page.h"
#include "../pages/wifi_page.h"
#include "../pages/bluetooth_page.h"

void nav_open_desktop() {
  desktop_create();
}

void nav_open_scan() {
  scan_page_create();
}

void nav_open_wifi() {
  wifi_page_create();
}

void nav_open_bluetooth() {
  bluetooth_page_create();
}

void nav_open_app(const char *title) {
  if (!title) return;

  if (strcmp(title, "Scan") == 0) {
    scan_page_create();
    return;
  }

  if (strcmp(title, "WiFi") == 0) {
    wifi_page_create();
    return;
  }

  if (strcmp(title, "Bluetooth") == 0) {
    bluetooth_page_create();
    return;
  }

  app_page_create(title);
}