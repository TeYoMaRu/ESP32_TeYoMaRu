#include "wifi_page.h"
#include "app_page.h"

#include <Arduino.h>
#include <WiFi.h>
#include <lvgl.h>
#include <cstring>

#include "../core/theme.h"

static lv_obj_t *wifi_list = nullptr;
static lv_obj_t *status_label = nullptr;
static lv_obj_t *password_area = nullptr;
static lv_obj_t *password_input = nullptr;
static lv_obj_t *keyboard = nullptr;

static String selected_ssid;

/* ======================================================
   แสดงสถานะ
====================================================== */

static void set_status(const char *text, lv_color_t color) {
  if (!status_label) return;

  lv_label_set_text(status_label, text);
  lv_obj_set_style_text_color(status_label, color, 0);
}

/* ======================================================
   ซ่อนช่องรหัสผ่านและ Keyboard
====================================================== */

static void hide_password_panel() {
  if (password_area) {
    lv_obj_add_flag(
      password_area,
      LV_OBJ_FLAG_HIDDEN
    );
  }

  if (keyboard) {
    lv_obj_add_flag(
      keyboard,
      LV_OBJ_FLAG_HIDDEN
    );
  }
}

/* ======================================================
   เชื่อมต่อ WiFi
====================================================== */

static void connect_event_cb(lv_event_t *event) {
  LV_UNUSED(event);

  if (!password_input) return;

  if (selected_ssid.length() == 0) {
    set_status(
      "Please select WiFi",
      COLOR_ORANGE
    );
    return;
  }

  const char *password =
    lv_textarea_get_text(password_input);

  set_status(
    "Connecting...",
    COLOR_ORANGE
  );

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(200);

  WiFi.begin(
    selected_ssid.c_str(),
    password
  );

  unsigned long start_time = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - start_time < 12000
  ) {
    delay(250);
    lv_timer_handler();
  }

  if (WiFi.status() == WL_CONNECTED) {
    String message = "Connected: ";
    message += selected_ssid;
    message += "\nIP: ";
    message += WiFi.localIP().toString();

    set_status(
      message.c_str(),
      COLOR_GREEN
    );

    hide_password_panel();
  } else {
    set_status(
      "Connection failed",
      C_RED
    );
  }
}

/* ======================================================
   เมื่อเลือก WiFi
====================================================== */

static void network_event_cb(lv_event_t *event) {
  const char *ssid =
    static_cast<const char *>(
      lv_event_get_user_data(event)
    );

  if (!ssid || !password_input) return;

  selected_ssid = ssid;

  lv_textarea_set_text(
    password_input,
    ""
  );

  if (password_area) {
    lv_obj_clear_flag(
      password_area,
      LV_OBJ_FLAG_HIDDEN
    );
  }

  if (keyboard) {
    lv_obj_clear_flag(
      keyboard,
      LV_OBJ_FLAG_HIDDEN
    );
  }

  String message = "Selected: ";
  message += selected_ssid;

  set_status(
    message.c_str(),
    COLOR_TEXT
  );
}

/* ======================================================
   คืนหน่วยความจำชื่อ SSID

   ssid_copy ถูกสร้างด้วย lv_mem_alloc()
   จึงต้องคืนเมื่อปุ่มถูกลบ
====================================================== */

static void network_delete_event_cb(lv_event_t *event) {
  char *ssid_copy =
    static_cast<char *>(
      lv_event_get_user_data(event)
    );

  if (ssid_copy) {
    lv_mem_free(ssid_copy);
  }
}

/* ======================================================
   สแกนเครือข่าย WiFi
====================================================== */

static void scan_wifi() {
  if (!wifi_list) return;

  lv_obj_clean(wifi_list);

  set_status(
    "Scanning WiFi...",
    COLOR_ORANGE
  );

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(200);

  int network_count = WiFi.scanNetworks();

  if (network_count <= 0) {
    set_status(
      "WiFi not found",
      C_RED
    );

    WiFi.scanDelete();
    return;
  }

  set_status(
    "Select WiFi network",
    COLOR_TEXT
  );

  for (int i = 0; i < network_count; i++) {
    String ssid = WiFi.SSID(i);

    if (ssid.length() == 0) {
      continue;
    }

    char *ssid_copy =
      static_cast<char *>(
        lv_mem_alloc(ssid.length() + 1)
      );

    if (!ssid_copy) {
      continue;
    }

    strcpy(
      ssid_copy,
      ssid.c_str()
    );

    lv_obj_t *button =
      lv_btn_create(wifi_list);

    lv_obj_set_width(
      button,
      lv_pct(96)
    );

    lv_obj_set_height(
      button,
      42
    );

    lv_obj_set_style_bg_color(
      button,
      COLOR_PANEL_2,
      0
    );

    lv_obj_set_style_border_width(
      button,
      1,
      0
    );

    lv_obj_set_style_border_color(
      button,
      COLOR_BORDER,
      0
    );

    lv_obj_set_style_radius(
      button,
      8,
      0
    );

    lv_obj_t *label =
      lv_label_create(button);

    String text = ssid;

    if (
      WiFi.encryptionType(i) !=
      WIFI_AUTH_OPEN
    ) {
      text += "  " LV_SYMBOL_EYE_CLOSE;
    }

    lv_label_set_text(
      label,
      text.c_str()
    );

    lv_obj_set_style_text_color(
      label,
      COLOR_TEXT,
      0
    );

    lv_obj_set_style_text_font(
      label,
      &lv_font_montserrat_14,
      0
    );

    lv_obj_align(
      label,
      LV_ALIGN_LEFT_MID,
      8,
      0
    );

    lv_obj_add_event_cb(
      button,
      network_event_cb,
      LV_EVENT_CLICKED,
      ssid_copy
    );

    lv_obj_add_event_cb(
      button,
      network_delete_event_cb,
      LV_EVENT_DELETE,
      ssid_copy
    );
  }

  WiFi.scanDelete();
}

/* ======================================================
   ปุ่ม Scan ใหม่
====================================================== */

static void rescan_event_cb(lv_event_t *event) {
  LV_UNUSED(event);

  hide_password_panel();

  selected_ssid = "";

  scan_wifi();
}

/* ======================================================
   ล้าง Pointer เมื่อหน้าถูกลบ
====================================================== */

static void screen_delete_event_cb(lv_event_t *event) {
  LV_UNUSED(event);

  wifi_list = nullptr;
  status_label = nullptr;
  password_area = nullptr;
  password_input = nullptr;
  keyboard = nullptr;
}

/* ======================================================
   สร้างหน้า WiFi
====================================================== */

void wifi_page_create() {
  lv_obj_t *screen = nullptr;

  /*
    ใช้โครงหน้ามาตรฐาน

    app_page_create_shell() จะสร้าง:
    - Screen
    - Status Bar
    - ชื่อหน้า
    - ปุ่ม Back
    - พื้นที่ Content
  */
  lv_obj_t *content =
    app_page_create_shell(
      LV_SYMBOL_WIFI "  WiFi",
      &screen
    );

  if (!screen || !content) {
    return;
  }

  lv_obj_add_event_cb(
    screen,
    screen_delete_event_cb,
    LV_EVENT_DELETE,
    nullptr
  );

  /* ====================================================
     ปุ่ม Scan ด้านขวาบน
  ==================================================== */

  lv_obj_t *scan_button =
    lv_btn_create(screen);

  lv_obj_set_size(
    scan_button,
    90,
    34
  );

  lv_obj_align(
    scan_button,
    LV_ALIGN_TOP_RIGHT,
    -12,
    33
  );

  lv_obj_set_style_bg_color(
    scan_button,
    COLOR_PRIMARY,
    0
  );

  lv_obj_set_style_radius(
    scan_button,
    8,
    0
  );

  lv_obj_add_event_cb(
    scan_button,
    rescan_event_cb,
    LV_EVENT_CLICKED,
    nullptr
  );

  lv_obj_t *scan_label =
    lv_label_create(scan_button);

  lv_label_set_text(
    scan_label,
    LV_SYMBOL_REFRESH " Scan"
  );

  lv_obj_set_style_text_color(
    scan_label,
    COLOR_BG,
    0
  );

  lv_obj_set_style_text_font(
    scan_label,
    &lv_font_montserrat_14,
    0
  );

  lv_obj_center(scan_label);

  /* ====================================================
     ข้อความสถานะ
  ==================================================== */

  status_label =
    lv_label_create(content);

  lv_label_set_text(
    status_label,
    "WiFi starting..."
  );

  lv_obj_set_width(
    status_label,
    440
  );

  lv_obj_set_style_text_align(
    status_label,
    LV_TEXT_ALIGN_CENTER,
    0
  );

  lv_obj_set_style_text_font(
    status_label,
    &lv_font_montserrat_14,
    0
  );

  lv_obj_align(
    status_label,
    LV_ALIGN_TOP_MID,
    0,
    0
  );

  /* ====================================================
     รายการเครือข่าย WiFi
  ==================================================== */

  wifi_list =
    lv_obj_create(content);

  lv_obj_set_size(
    wifi_list,
    440,
    190
  );

  lv_obj_align(
    wifi_list,
    LV_ALIGN_BOTTOM_MID,
    0,
    0
  );

  lv_obj_set_flex_flow(
    wifi_list,
    LV_FLEX_FLOW_COLUMN
  );

  lv_obj_set_flex_align(
    wifi_list,
    LV_FLEX_ALIGN_START,
    LV_FLEX_ALIGN_CENTER,
    LV_FLEX_ALIGN_CENTER
  );

  lv_obj_set_style_bg_color(
    wifi_list,
    COLOR_PANEL,
    0
  );

  lv_obj_set_style_border_color(
    wifi_list,
    COLOR_BORDER,
    0
  );

  lv_obj_set_style_border_width(
    wifi_list,
    1,
    0
  );

  lv_obj_set_style_radius(
    wifi_list,
    12,
    0
  );

  lv_obj_set_style_pad_all(
    wifi_list,
    8,
    0
  );

  lv_obj_set_style_pad_row(
    wifi_list,
    6,
    0
  );

  /* ====================================================
     Panel ใส่รหัสผ่าน

     วางบนพื้นที่ Content
  ==================================================== */

  password_area =
    lv_obj_create(content);

  lv_obj_set_size(
    password_area,
    440,
    70
  );

  lv_obj_align(
    password_area,
    LV_ALIGN_TOP_MID,
    0,
    28
  );

  theme_apply_panel(
    password_area
  );

  lv_obj_add_flag(
    password_area,
    LV_OBJ_FLAG_HIDDEN
  );

  password_input =
    lv_textarea_create(password_area);

  lv_obj_set_size(
    password_input,
    295,
    42
  );

  lv_obj_align(
    password_input,
    LV_ALIGN_LEFT_MID,
    0,
    0
  );

  lv_textarea_set_placeholder_text(
    password_input,
    "WiFi password"
  );

  lv_textarea_set_password_mode(
    password_input,
    true
  );

  lv_textarea_set_one_line(
    password_input,
    true
  );

  lv_obj_t *connect_button =
    lv_btn_create(password_area);

  lv_obj_set_size(
    connect_button,
    110,
    42
  );

  lv_obj_align(
    connect_button,
    LV_ALIGN_RIGHT_MID,
    0,
    0
  );

  lv_obj_set_style_bg_color(
    connect_button,
    COLOR_PRIMARY,
    0
  );

  lv_obj_add_event_cb(
    connect_button,
    connect_event_cb,
    LV_EVENT_CLICKED,
    nullptr
  );

  lv_obj_t *connect_label =
    lv_label_create(connect_button);

  lv_label_set_text(
    connect_label,
    "Connect"
  );

  lv_obj_set_style_text_color(
    connect_label,
    COLOR_BG,
    0
  );

  lv_obj_center(connect_label);

  /* ====================================================
     Keyboard

     ต้องเป็นลูกของ Screen เพื่อแสดงเต็มความกว้าง
  ==================================================== */

  keyboard =
    lv_keyboard_create(screen);

  lv_obj_set_size(
    keyboard,
    480,
    145
  );

  lv_obj_align(
    keyboard,
    LV_ALIGN_BOTTOM_MID,
    0,
    0
  );

  lv_keyboard_set_textarea(
    keyboard,
    password_input
  );

  lv_obj_add_flag(
    keyboard,
    LV_OBJ_FLAG_HIDDEN
  );

  /* โหลดหน้า WiFi */
  lv_scr_load(screen);

  /* เริ่มสแกนหลังโหลดหน้า */
  scan_wifi();
}