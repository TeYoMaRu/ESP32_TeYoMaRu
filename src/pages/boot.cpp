#include "boot.h"
#include "desktop.h"
#include "../core/theme.h"
#include <cstdio>

// Object ของหน้า Boot
static lv_obj_t *boot_bar = NULL;
static lv_obj_t *percent_label = NULL;
static lv_obj_t *status_label = NULL;
static lv_timer_t *boot_timer = NULL;
static int boot_percent = 0;

/*
   Timer เพิ่มเปอร์เซ็นต์ของ Boot
*/
static void boot_timer_cb(lv_timer_t *timer) {
  boot_percent += 2;

  if (boot_percent > 100) {
    boot_percent = 100;
  }

  lv_bar_set_value(boot_bar, boot_percent, LV_ANIM_ON);

  static char buf[16];
  snprintf(buf, sizeof(buf), "%d%%", boot_percent);
  lv_label_set_text(percent_label, buf);

  if (boot_percent == 25) {
    lv_label_set_text(status_label, "Loading TeYoMaRu core...");
  } else if (boot_percent == 55) {
    lv_label_set_text(status_label, "Preparing desktop...");
  } else if (boot_percent == 85) {
    lv_label_set_text(status_label, "Almost ready...");
  } else if (boot_percent >= 100) {
    lv_label_set_text(status_label, "Boot complete");

    lv_timer_del(timer);
    boot_timer = NULL;

    desktop_create();
  }
}

/*
   สร้างหน้า Boot
*/
void boot_create() {
  lv_obj_t *screen = lv_scr_act();

  lv_obj_set_style_bg_color(screen, C_BG, 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  // Panel กลางจอ
  lv_obj_t *panel = lv_obj_create(screen);
  lv_obj_set_size(panel, 400, 220);
  lv_obj_center(panel);

  lv_obj_set_style_radius(panel, 18, 0);
  lv_obj_set_style_bg_color(panel, C_PANEL, 0);
  lv_obj_set_style_border_color(panel, C_BLUE, 0);
  lv_obj_set_style_border_width(panel, 2, 0);
  lv_obj_set_style_shadow_width(panel, 24, 0);
  lv_obj_set_style_shadow_color(panel, C_BLUE_DARK, 0);

  // Title
  lv_obj_t *title = lv_label_create(panel);
  lv_label_set_text(title, "TeYoMaRu OS");
  lv_obj_set_style_text_color(title, C_BLUE, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

  // Status
  status_label = lv_label_create(panel);
  lv_label_set_text(status_label, "Starting display engine...");
  lv_obj_set_style_text_color(status_label, lv_color_hex(0xE5E7EB), 0);
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_16, 0);
  lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 78);

  // Progress bar
  boot_bar = lv_bar_create(panel);
  lv_obj_set_size(boot_bar, 300, 18);
  lv_obj_align(boot_bar, LV_ALIGN_TOP_MID, 0, 128);
  lv_bar_set_range(boot_bar, 0, 100);
  lv_bar_set_value(boot_bar, 0, LV_ANIM_OFF);

  lv_obj_set_style_radius(boot_bar, 9, LV_PART_MAIN);
  lv_obj_set_style_radius(boot_bar, 9, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(boot_bar, lv_color_hex(0x1F2937), LV_PART_MAIN);
  lv_obj_set_style_bg_color(boot_bar, C_BLUE, LV_PART_INDICATOR);

  // Percent
  percent_label = lv_label_create(panel);
  lv_label_set_text(percent_label, "0%");
  lv_obj_set_style_text_color(percent_label, C_TEXT, 0);
  lv_obj_set_style_text_font(percent_label, &lv_font_montserrat_18, 0);
  lv_obj_align(percent_label, LV_ALIGN_TOP_MID, 0, 158);
}

/*
   เริ่ม Boot Timer
*/
void boot_start() {
  boot_percent = 0;
  boot_timer = lv_timer_create(boot_timer_cb, 80, NULL);
}