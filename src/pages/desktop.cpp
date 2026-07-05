#include "desktop.h"
#include "../core/theme.h"
#include "../widgets/widgets.h"
#include <cstring>

static lv_obj_t *desktop_status_label = NULL;

static void on_tile_clicked(const char *title)
{
    if (!desktop_status_label)
        return;

    if (strcmp(title, "SCAN") == 0)
    {
        lv_label_set_text(desktop_status_label, "SCAN selected");
    }
    else if (strcmp(title, "WIFI") == 0)
    {
        lv_label_set_text(desktop_status_label, "WIFI selected");
    }
    else if (strcmp(title, "SYS") == 0)
    {
        lv_label_set_text(desktop_status_label, "SYS selected");
    }
}

/*
   สร้างหน้า Desktop หลัก
*/
void desktop_create()
{
    lv_obj_t *desktop = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(desktop, C_BG, 0);
    lv_obj_set_style_bg_opa(desktop, LV_OPA_COVER, 0);

    // Topbar
    lv_obj_t *topbar = lv_obj_create(desktop);
    lv_obj_set_size(topbar, 480, 48);
    lv_obj_set_pos(topbar, 0, 0);
    lv_obj_set_style_radius(topbar, 0, 0);
    lv_obj_set_style_bg_color(topbar, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_border_width(topbar, 0, 0);

    lv_obj_t *title = lv_label_create(topbar);
    lv_label_set_text(title, "TeYoMaRu OS");
    lv_obj_set_style_text_color(title, C_BLUE, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 16, 0);

    desktop_status_label = lv_label_create(topbar);
    lv_label_set_text(desktop_status_label, "READY");
    lv_obj_set_style_text_color(desktop_status_label, C_GREEN, 0);
    lv_obj_set_style_text_font(desktop_status_label, &lv_font_montserrat_14, 0);
    lv_obj_align(desktop_status_label, LV_ALIGN_RIGHT_MID, -16, 0);

    // Panel หลัก
    lv_obj_t *card = ui_create_panel(desktop, 16, 62, 448, 245);

    lv_obj_t *welcome = lv_label_create(card);
    lv_label_set_text(welcome, "Welcome back");
    lv_obj_set_style_text_color(welcome, C_TEXT, 0);
    lv_obj_set_style_text_font(welcome, &lv_font_montserrat_24, 0);
    lv_obj_align(welcome, LV_ALIGN_TOP_LEFT, 18, 16);

    lv_obj_t *sub = lv_label_create(card);
    lv_label_set_text(sub, "Choose a module to continue");
    lv_obj_set_style_text_color(sub, C_MUTED, 0);
    lv_obj_set_style_text_font(sub, &lv_font_montserrat_14, 0);
    lv_obj_align(sub, LV_ALIGN_TOP_LEFT, 18, 48);

    ui_create_tile(card, 18, 92, "SCAN", "Device scan", on_tile_clicked);
    ui_create_tile(card, 158, 92, "WIFI", "Network", on_tile_clicked);
    ui_create_tile(card, 298, 92, "SYS", "System info", on_tile_clicked);

    lv_obj_t *footer = lv_label_create(card);
    lv_label_set_text(footer, "ESP32 + ILI9488 + LVGL");
    lv_obj_set_style_text_color(footer, lv_color_hex(0x64748B), 0);
    lv_obj_set_style_text_font(footer, &lv_font_montserrat_12, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -12);

    
    lv_scr_load_anim(desktop, LV_SCR_LOAD_ANIM_FADE_ON, 250, 0, true);
}