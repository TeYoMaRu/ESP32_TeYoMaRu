#include "wifi_page.h"

#include <Arduino.h>
#include <WiFi.h>
#include <lvgl.h>

#include "../core/theme.h"
#include "../app/navigation.h"

static lv_obj_t *wifi_list;
static lv_obj_t *status_label;
static lv_obj_t *password_area;
static lv_obj_t *password_input;
static lv_obj_t *keyboard;

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
   กลับหน้า Desktop
====================================================== */

static void back_event_cb(lv_event_t *event) {
  LV_UNUSED(event);
  nav_open_desktop();
}

/* ======================================================
   ซ่อนช่องรหัสผ่าน
====================================================== */

static void hide_password_panel() {
  if (password_area) {
    lv_obj_add_flag(password_area, LV_OBJ_FLAG_HIDDEN);
  }

  if (keyboard) {
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  }
}

/* ======================================================
   เชื่อมต่อ WiFi
====================================================== */

static void connect_event_cb(lv_event_t *event) {
  LV_UNUSED(event);

  if (selected_ssid.length() == 0) {
    set_status("Please select WiFi", COLOR_ORANGE);
    return;
  }

  const char *password = lv_textarea_get_text(password_input);

  set_status("Connecting...", COLOR_ORANGE);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(200);

  WiFi.begin(selected_ssid.c_str(), password);

  unsigned long start_time = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start_time < 12000) {
    delay(250);
    lv_timer_handler();
  }

  if (WiFi.status() == WL_CONNECTED) {
    String message = "Connected: ";
    message += selected_ssid;
    message += "\nIP: ";
    message += WiFi.localIP().toString();

    set_status(message.c_str(), COLOR_GREEN);
    hide_password_panel();
  } else {
    set_status("Connection failed", C_RED);
  }
}

/* ======================================================
   เมื่อเลือก WiFi
====================================================== */

static void network_event_cb(lv_event_t *event) {
  lv_obj_t *button = lv_event_get_target(event);
  const char *ssid = static_cast<const char *>(
    lv_event_get_user_data(event)
  );

  if (!ssid) return;

  selected_ssid = ssid;

  lv_textarea_set_text(password_input, "");

  lv_obj_clear_flag(password_area, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);

  String message = "Selected: ";
  message += selected_ssid;

  set_status(message.c_str(), COLOR_TEXT);

  LV_UNUSED(button);
}

/* ======================================================
   สแกนเครือข่าย WiFi
====================================================== */

static void scan_wifi() {
  lv_obj_clean(wifi_list);

  set_status("Scanning WiFi...", COLOR_ORANGE);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(200);

  int network_count = WiFi.scanNetworks();

  if (network_count <= 0) {
    set_status("WiFi not found", C_RED);
    return;
  }

  set_status("Select WiFi network", COLOR_TEXT);

  for (int i = 0; i < network_count; i++) {
    String ssid = WiFi.SSID(i);

    if (ssid.length() == 0) continue;

    char *ssid_copy = static_cast<char *>(
      lv_mem_alloc(ssid.length() + 1)
    );

    if (!ssid_copy) continue;

    strcpy(ssid_copy, ssid.c_str());

    lv_obj_t *button = lv_btn_create(wifi_list);
    lv_obj_set_width(button, lv_pct(96));
    lv_obj_set_height(button, 42);

    lv_obj_set_style_bg_color(button, COLOR_PANEL_2, 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_border_color(button, COLOR_BORDER, 0);
    lv_obj_set_style_radius(button, 8, 0);

    lv_obj_t *label = lv_label_create(button);

    String text = ssid;

    if (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) {
      text += "  " LV_SYMBOL_EYE_CLOSE;
    }

    lv_label_set_text(label, text.c_str());
    lv_obj_set_style_text_color(label, COLOR_TEXT, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_add_event_cb(
      button,
      network_event_cb,
      LV_EVENT_CLICKED,
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
   สร้างหน้า WiFi
====================================================== */

void wifi_page_create() {
  lv_obj_t *screen = lv_obj_create(NULL);
  theme_apply_screen(screen);

  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  /* หัวข้อ */
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, LV_SYMBOL_WIFI "  WiFi");
  theme_apply_label(title, true);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

  /* ปุ่ม Back */
  lv_obj_t *back_button = lv_btn_create(screen);
  lv_obj_set_size(back_button, 85, 38);
  lv_obj_align(back_button, LV_ALIGN_TOP_LEFT, 12, 8);
  lv_obj_set_style_bg_color(back_button, COLOR_PANEL_2, 0);
  lv_obj_add_event_cb(
    back_button,
    back_event_cb,
    LV_EVENT_CLICKED,
    nullptr
  );

  lv_obj_t *back_label = lv_label_create(back_button);
  lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
  lv_obj_center(back_label);

  /* ปุ่ม Scan */
  lv_obj_t *scan_button = lv_btn_create(screen);
  lv_obj_set_size(scan_button, 90, 38);
  lv_obj_align(scan_button, LV_ALIGN_TOP_RIGHT, -12, 8);
  lv_obj_set_style_bg_color(scan_button, COLOR_PRIMARY, 0);
  lv_obj_add_event_cb(
    scan_button,
    rescan_event_cb,
    LV_EVENT_CLICKED,
    nullptr
  );

  lv_obj_t *scan_label = lv_label_create(scan_button);
  lv_label_set_text(scan_label, LV_SYMBOL_REFRESH " Scan");
  lv_obj_set_style_text_color(scan_label, lv_color_black(), 0);
  lv_obj_center(scan_label);

  /* สถานะ */
  status_label = lv_label_create(screen);
  lv_label_set_text(status_label, "WiFi starting...");
  lv_obj_set_width(status_label, 440);
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
  lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 53);

  /* รายการ WiFi */
  wifi_list = lv_obj_create(screen);
  lv_obj_set_size(wifi_list, 440, 210);
  lv_obj_align(wifi_list, LV_ALIGN_TOP_MID, 0, 83);

  lv_obj_set_flex_flow(wifi_list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(
    wifi_list,
    LV_FLEX_ALIGN_START,
    LV_FLEX_ALIGN_CENTER,
    LV_FLEX_ALIGN_CENTER
  );

  lv_obj_set_style_bg_color(wifi_list, COLOR_PANEL, 0);
  lv_obj_set_style_border_color(wifi_list, COLOR_BORDER, 0);
  lv_obj_set_style_border_width(wifi_list, 1, 0);
  lv_obj_set_style_radius(wifi_list, 12, 0);
  lv_obj_set_style_pad_all(wifi_list, 8, 0);
  lv_obj_set_style_pad_row(wifi_list, 6, 0);

  /* Panel ใส่รหัสผ่าน */
  password_area = lv_obj_create(screen);
  lv_obj_set_size(password_area, 440, 95);
  lv_obj_align(password_area, LV_ALIGN_BOTTOM_MID, 0, -5);
  theme_apply_panel(password_area);
  lv_obj_add_flag(password_area, LV_OBJ_FLAG_HIDDEN);

  password_input = lv_textarea_create(password_area);
  lv_obj_set_size(password_input, 300, 42);
  lv_obj_align(password_input, LV_ALIGN_LEFT_MID, 0, 0);
  lv_textarea_set_placeholder_text(
    password_input,
    "WiFi password"
  );
  lv_textarea_set_password_mode(password_input, true);
  lv_textarea_set_one_line(password_input, true);

  lv_obj_t *connect_button = lv_btn_create(password_area);
  lv_obj_set_size(connect_button, 110, 42);
  lv_obj_align(connect_button, LV_ALIGN_RIGHT_MID, 0, 0);
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

  lv_obj_t *connect_label = lv_label_create(connect_button);
  lv_label_set_text(connect_label, "Connect");
  lv_obj_set_style_text_color(
    connect_label,
    lv_color_black(),
    0
  );
  lv_obj_center(connect_label);

  /* Keyboard */
  keyboard = lv_keyboard_create(screen);
  lv_obj_set_size(keyboard, 480, 150);
  lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_keyboard_set_textarea(keyboard, password_input);
  lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);

  lv_scr_load(screen);

  scan_wifi();
}