#include "bluetooth_page.h"

#include <Arduino.h>
#include <BluetoothSerial.h>
#include <lvgl.h>

#include "../core/theme.h"
#include "../app/navigation.h"

#if !defined(CONFIG_BT_ENABLED) || \
    !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled for this ESP32 board
#endif

static BluetoothSerial serial_bt;

static lv_obj_t *bt_status_label;
static lv_obj_t *bt_switch;

static bool bluetooth_started = false;

/* ======================================================
   อัปเดตข้อความสถานะ
====================================================== */

static void update_bluetooth_status() {
  if (!bt_status_label) return;

  if (bluetooth_started) {
    lv_label_set_text(
      bt_status_label,
      "Bluetooth: ON\nDevice name: TeYoMaRu-ESP32"
    );

    lv_obj_set_style_text_color(
      bt_status_label,
      COLOR_GREEN,
      0
    );
  } else {
    lv_label_set_text(
      bt_status_label,
      "Bluetooth: OFF"
    );

    lv_obj_set_style_text_color(
      bt_status_label,
      COLOR_MUTED,
      0
    );
  }
}

/* ======================================================
   เปิด Bluetooth
====================================================== */

static void bluetooth_start() {
  if (bluetooth_started) return;

  bluetooth_started = serial_bt.begin("TeYoMaRu-ESP32");

  update_bluetooth_status();
}

/* ======================================================
   ปิด Bluetooth
====================================================== */

static void bluetooth_stop() {
  if (!bluetooth_started) return;

  serial_bt.end();
  bluetooth_started = false;

  update_bluetooth_status();
}

/* ======================================================
   Event สวิตช์
====================================================== */

static void switch_event_cb(lv_event_t *event) {
  lv_obj_t *switch_object = lv_event_get_target(event);

  bool checked = lv_obj_has_state(
    switch_object,
    LV_STATE_CHECKED
  );

  if (checked) {
    bluetooth_start();
  } else {
    bluetooth_stop();
  }
}

/* ======================================================
   ปุ่ม Back
====================================================== */

static void back_event_cb(lv_event_t *event) {
  LV_UNUSED(event);
  nav_open_desktop();
}

/* ======================================================
   สร้างหน้า Bluetooth
====================================================== */

void bluetooth_page_create() {
  lv_obj_t *screen = lv_obj_create(NULL);
  theme_apply_screen(screen);

  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  /* หัวข้อ */
  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(
    title,
    LV_SYMBOL_BLUETOOTH "  Bluetooth"
  );
  theme_apply_label(title, true);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

  /* ปุ่ม Back */
  lv_obj_t *back_button = lv_btn_create(screen);
  lv_obj_set_size(back_button, 85, 40);
  lv_obj_align(back_button, LV_ALIGN_TOP_LEFT, 14, 12);
  lv_obj_set_style_bg_color(
    back_button,
    COLOR_PANEL_2,
    0
  );

  lv_obj_add_event_cb(
    back_button,
    back_event_cb,
    LV_EVENT_CLICKED,
    nullptr
  );

  lv_obj_t *back_label = lv_label_create(back_button);
  lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
  lv_obj_center(back_label);

  /* กล่อง Bluetooth */
  lv_obj_t *panel = lv_obj_create(screen);
  lv_obj_set_size(panel, 400, 190);
  lv_obj_align(panel, LV_ALIGN_CENTER, 0, 15);
  theme_apply_panel(panel);

  /* ไอคอน */
  lv_obj_t *icon = lv_label_create(panel);
  lv_label_set_text(icon, LV_SYMBOL_BLUETOOTH);
  lv_obj_set_style_text_font(
    icon,
    &lv_font_montserrat_20,
    0
  );
  lv_obj_set_style_text_color(icon, COLOR_PRIMARY, 0);
  lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 10);

  /* ชื่ออุปกรณ์ */
  lv_obj_t *name_label = lv_label_create(panel);
  lv_label_set_text(name_label, "TeYoMaRu-ESP32");
  lv_obj_set_style_text_font(
    name_label,
    &lv_font_montserrat_18,
    0
  );
  lv_obj_set_style_text_color(
    name_label,
    COLOR_TEXT,
    0
  );
  lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 45);

  /* สวิตช์ */
  bt_switch = lv_switch_create(panel);
  lv_obj_set_size(bt_switch, 65, 32);
  lv_obj_align(bt_switch, LV_ALIGN_CENTER, 0, 15);

  if (bluetooth_started) {
    lv_obj_add_state(bt_switch, LV_STATE_CHECKED);
  }

  lv_obj_add_event_cb(
    bt_switch,
    switch_event_cb,
    LV_EVENT_VALUE_CHANGED,
    nullptr
  );

  /* สถานะ */
  bt_status_label = lv_label_create(panel);
  lv_obj_set_width(bt_status_label, 360);
  lv_obj_set_style_text_align(
    bt_status_label,
    LV_TEXT_ALIGN_CENTER,
    0
  );
  lv_obj_set_style_text_font(
    bt_status_label,
    &lv_font_montserrat_14,
    0
  );
  lv_obj_align(
    bt_status_label,
    LV_ALIGN_BOTTOM_MID,
    0,
    -15
  );

  update_bluetooth_status();

  lv_scr_load(screen);
}