#include <cstring>
#include "navigation.h"
#include "../pages/desktop.h"
#include "../pages/app_page.h"
#include "../pages/scan_page.h"

void nav_open_desktop() {
  desktop_create();
}

void nav_open_scan() {
  scan_page_create();
}

void nav_open_app(const char *title) {
  // เมนู Scan เปิดหน้า QR โดยตรง
  if (strcmp(title, "Scan") == 0) {
    scan_page_create();
    return;
  }
  app_page_create(title);
}
