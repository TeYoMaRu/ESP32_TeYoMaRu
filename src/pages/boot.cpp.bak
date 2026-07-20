#include "boot.h"
#include "desktop.h"
#include "../core/theme.h"
#include <lvgl.h>
#include <cstdio>

/* ======================================================
   Boot Screen
   จุดสำคัญของชุดนี้:
   - ไม่มี Fade ตอนเข้า Desktop เพื่อลดอาการกระตุก
   - ใช้ lv_bar แบบ LV_ANIM_OFF เพื่อให้ลื่นกว่า
   - สีหลักเปลี่ยนเป็น เหลือง / ส้ม / ขาว / แดง
====================================================== */

static lv_obj_t *boot_scr = NULL;
static lv_obj_t *boot_bar = NULL;
static lv_obj_t *percent_label = NULL;
static lv_obj_t *info_label = NULL;
static lv_timer_t *boot_timer = NULL;
static int boot_value = 0;

static const char *step_text(int value) {
  if (value < 20) return "Starting...";
  if (value < 45) return "Loading System...";
  if (value < 70) return "Loading GUI...";
  if (value < 100) return "Almost Ready...";
  return "System Ready";
}

static void boot_timer_cb(lv_timer_t *timer) {
  boot_value += 4;
  if (boot_value > 100) boot_value = 100;

  lv_bar_set_value(boot_bar, boot_value, LV_ANIM_OFF);

  char buf[8];
  snprintf(buf, sizeof(buf), "%d%%", boot_value);
  lv_label_set_text(percent_label, buf);
  lv_label_set_text(info_label, step_text(boot_value));

  if (boot_value >= 100) {
    lv_timer_del(timer);
    boot_timer = NULL;

    // หน่วงนิดเดียวให้เห็นคำว่า System Ready
    lv_timer_create([](lv_timer_t *t) {
      lv_timer_del(t);
      desktop_create();  // โหลดตรง ๆ ไม่ใช้ Fade
    }, 350, NULL);
  }
}

void boot_create() {
  boot_scr = lv_obj_create(NULL);
  theme_apply_screen(boot_scr);

  lv_obj_t *title = lv_label_create(boot_scr);
  lv_label_set_text(title, "TeYoMaRu OS");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(title, COLOR_TEXT, 0);
  lv_obj_align(title, LV_ALIGN_CENTER, 0, -45);

  lv_obj_t *os = lv_label_create(boot_scr);
  lv_label_set_text(os, "OS");
  lv_obj_set_style_text_font(os, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(os, COLOR_PRIMARY, 0);
  lv_obj_align_to(os, title, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

  percent_label = lv_label_create(boot_scr);
  lv_label_set_text(percent_label, "0%");
  theme_apply_label(percent_label, false);
  lv_obj_align(percent_label, LV_ALIGN_CENTER, 0, 10);

  boot_bar = lv_bar_create(boot_scr);
  lv_obj_set_size(boot_bar, 225, 12);
  lv_obj_align(boot_bar, LV_ALIGN_CENTER, 0, 38);
  lv_bar_set_range(boot_bar, 0, 100);
  lv_bar_set_value(boot_bar, 0, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(boot_bar, COLOR_PANEL, 0);
  lv_obj_set_style_border_color(boot_bar, COLOR_BORDER, 0);
  lv_obj_set_style_border_width(boot_bar, 1, 0);
  lv_obj_set_style_radius(boot_bar, 4, 0);
  lv_obj_set_style_bg_color(boot_bar, COLOR_PRIMARY, LV_PART_INDICATOR);
  lv_obj_set_style_radius(boot_bar, 3, LV_PART_INDICATOR);

  info_label = lv_label_create(boot_scr);
  lv_label_set_text(info_label, "Starting...");
  theme_apply_small_label(info_label, false);
  lv_obj_set_style_text_color(info_label, COLOR_ORANGE, 0);
  lv_obj_align(info_label, LV_ALIGN_CENTER, 0, 68);

  lv_obj_t *dots = lv_label_create(boot_scr);
  lv_label_set_text(dots, ".  .  .");
  lv_obj_set_style_text_color(dots, COLOR_PRIMARY, 0);
  lv_obj_set_style_text_font(dots, &lv_font_montserrat_18, 0);
  lv_obj_align(dots, LV_ALIGN_BOTTOM_MID, 0, -14);
}

void boot_start() {
  boot_value = 0;
  lv_scr_load(boot_scr);

  if (boot_timer != NULL) {
    lv_timer_del(boot_timer);
    boot_timer = NULL;
  }

  boot_timer = lv_timer_create(boot_timer_cb, 80, NULL);
}
