#include "navigation.h"
#include "../pages/desktop.h"
#include "../pages/app_page.h"

void nav_open_desktop() {
  desktop_create();
}

void nav_open_app(const char *title) {
  app_page_create(title);
}
