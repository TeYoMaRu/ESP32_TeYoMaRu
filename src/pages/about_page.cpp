#include "about_page.h"
#include "app_page.h"

#include <Arduino.h>
#include <lvgl.h>

#include "../core/theme.h"

/* ======================================================
   สร้างแถวข้อมูล
====================================================== */

static void create_info_row(
    lv_obj_t *parent,
    const char *title,
    const char *value
) {
    lv_obj_t *title_label = lv_label_create(parent);
    lv_label_set_text(title_label, title);
    theme_apply_small_label(title_label, false);

    lv_obj_set_width(title_label, lv_pct(100));
    lv_obj_set_style_pad_left(title_label, 4, 0);
    lv_obj_set_style_pad_top(title_label, 4, 0);

    lv_obj_t *value_label = lv_label_create(parent);
    lv_label_set_text(value_label, value);
    theme_apply_label(value_label, false);

    lv_obj_set_width(value_label, lv_pct(100));
    lv_obj_set_style_pad_left(value_label, 4, 0);
    lv_obj_set_style_pad_bottom(value_label, 8, 0);
}

/* ======================================================
   สร้างหน้า About
====================================================== */

void about_page_create() {
    lv_obj_t *screen = nullptr;

    lv_obj_t *content =
        app_page_create_shell(
            LV_SYMBOL_FILE "  About",
            &screen
        );

    if (!screen || !content) {
        return;
    }

    /* จัดรายการจากบนลงล่าง */
    lv_obj_set_flex_flow(
        content,
        LV_FLEX_FLOW_COLUMN
    );

    lv_obj_set_flex_align(
        content,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    lv_obj_set_style_pad_all(content, 10, 0);
    lv_obj_set_style_pad_row(content, 2, 0);

    /* ตั้งค่าการเลื่อน */
    lv_obj_add_flag(
        content,
        LV_OBJ_FLAG_SCROLLABLE
    );

    lv_obj_set_scroll_dir(
        content,
        LV_DIR_VER
    );

    /*
       ปิดแรงเหวี่ยงและการเด้งขอบ
       ช่วยลดอาการกระตุกบนจอ ILI9488
    */
    lv_obj_clear_flag(
        content,
        LV_OBJ_FLAG_SCROLL_MOMENTUM
    );

    lv_obj_clear_flag(
        content,
        LV_OBJ_FLAG_SCROLL_ELASTIC
    );

    /*
       ปิด Scrollbar เพื่อลดพื้นที่ที่ LVGL
       ต้องวาดใหม่ระหว่างเลื่อน
    */
    lv_obj_set_scrollbar_mode(
        content,
        LV_SCROLLBAR_MODE_OFF
    );

    /* ชื่อระบบ */
    lv_obj_t *name_label =
        lv_label_create(content);

    lv_label_set_text(
        name_label,
        "TeYoMaRu OS"
    );

    theme_apply_label(
        name_label,
        true
    );

    lv_obj_set_style_text_font(
        name_label,
        &lv_font_montserrat_24,
        0
    );

    lv_obj_set_style_pad_bottom(
        name_label,
        8,
        0
    );

    /* ข้อมูลระบบ */
    create_info_row(
        content,
        "Version",
        "0.3.3 Alpha"
    );

    create_info_row(
        content,
        "Board",
        "ESP32 DevKit V1"
    );

    create_info_row(
        content,
        "Display",
        "ILI9488 480x320"
    );

    create_info_row(
        content,
        "LVGL",
        LVGL_VERSION_INFO
    );

    char ram_text[16];

    snprintf(
        ram_text,
        sizeof(ram_text),
        "%u KB",
        static_cast<unsigned int>(
            ESP.getFreeHeap() / 1024
        )
    );

    create_info_row(
        content,
        "Free RAM",
        ram_text
    );

    create_info_row(
        content,
        "Build",
        __DATE__ "  " __TIME__
    );

    lv_scr_load(screen);
}