#include "boot.h"
#include "desktop.h"
#include "../core/theme.h"
#include "../assets/teyomaru_logo.h"


#include <lvgl.h>
#include <cstdio>

/* ======================================================
   TeYoMaRu OS - Cyber Boot Screen
   สำหรับจอ 480 x 320 แนวนอน

   รูปแบบ:
   - พื้นหลังสีดำ
   - เส้นกรอบ Cyber สีแดง
   - โลโก้ TeYoMaRu จากไฟล์ assets
   - แสดงสถานะระบบทีละรายการ
   - Progress Bar สีแดง

   หมายเหตุ:
   - ใช้ชื่อสีขึ้นต้นด้วย BOOT_
     เพื่อไม่ให้ซ้ำกับ Macro ใน theme.h
   - รองรับ LVGL 8.4.0
====================================================== */

/* ======================================================
   Object หลักของหน้าบูท
====================================================== */

static lv_obj_t *boot_scr = NULL;

static lv_obj_t *boot_bar = NULL;
static lv_obj_t *percent_label = NULL;
static lv_obj_t *boot_message = NULL;

/* Label แสดงสถานะระบบ */
static lv_obj_t *status_display = NULL;
static lv_obj_t *status_touch = NULL;
static lv_obj_t *status_ui = NULL;
static lv_obj_t *status_memory = NULL;
static lv_obj_t *status_pages = NULL;

/* Timer สำหรับอัปเดต Progress */
static lv_timer_t *boot_timer = NULL;

/* ค่าเปอร์เซ็นต์ปัจจุบัน */
static int boot_value = 0;

/* ======================================================
   สีเฉพาะของหน้าบูท

   ห้ามใช้ชื่อ C_BG, C_RED, C_GREEN ฯลฯ
   เพราะชื่อเหล่านี้มีอยู่แล้วใน theme.h
====================================================== */

static const lv_color_t BOOT_BG =
    lv_color_hex(0x030303);

static const lv_color_t BOOT_PANEL =
    lv_color_hex(0x080808);

static const lv_color_t BOOT_RED =
    lv_color_hex(0xFF2020);

static const lv_color_t BOOT_RED_DARK =
    lv_color_hex(0x6E0808);

static const lv_color_t BOOT_WHITE =
    lv_color_hex(0xF4F4F4);

static const lv_color_t BOOT_GREY =
    lv_color_hex(0x888888);

static const lv_color_t BOOT_GREEN =
    lv_color_hex(0x35FF4D);

static const lv_color_t BOOT_YELLOW =
    lv_color_hex(0xFFD000);

/* ======================================================
   จุดของเส้นตกแต่ง

   ต้องประกาศเป็น static
   เพราะ lv_line จะเก็บ pointer ของข้อมูลไว้
====================================================== */

/* เส้นมุมซ้ายบน */
static lv_point_t line_top_left[] = {
    {6, 28},
    {26, 28},
    {52, 52},
    {100, 52}
};

/* เส้นมุมขวาบน */
static lv_point_t line_top_right[] = {
    {374, 52},
    {428, 52},
    {452, 28},
    {474, 28}
};

/* เส้นมุมซ้ายล่าง */
static lv_point_t line_bottom_left[] = {
    {6, 291},
    {26, 291},
    {40, 306},
    {104, 306}
};

/* เส้นมุมขวาล่าง */
static lv_point_t line_bottom_right[] = {
    {376, 306},
    {440, 306},
    {454, 291},
    {474, 291}
};

/* ======================================================
   สร้าง Label แบบกำหนด Font และสี
====================================================== */

static lv_obj_t *create_label(
    lv_obj_t *parent,
    const char *text,
    const lv_font_t *font,
    lv_color_t color
) {
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);

    lv_obj_set_style_text_font(
        label,
        font,
        0
    );

    lv_obj_set_style_text_color(
        label,
        color,
        0
    );

    lv_obj_set_style_text_letter_space(
        label,
        1,
        0
    );

    return label;
}

/* ======================================================
   สร้างเส้น Cyber
====================================================== */

static lv_obj_t *create_line(
    lv_obj_t *parent,
    lv_point_t points[],
    uint16_t point_count,
    lv_color_t color,
    int width
) {
    lv_obj_t *line = lv_line_create(parent);

    lv_line_set_points(
        line,
        points,
        point_count
    );

    lv_obj_set_style_line_color(
        line,
        color,
        0
    );

    lv_obj_set_style_line_width(
        line,
        width,
        0
    );

    lv_obj_set_style_line_rounded(
        line,
        true,
        0
    );

    return line;
}

/* ======================================================
   สร้างแถวแสดงสถานะระบบ

   ตัวอย่าง:
   ● DISPLAY                     WAIT
====================================================== */

static lv_obj_t *create_status_row(
    lv_obj_t *parent,
    int y,
    const char *name
) {
    /* เส้นแบ่งใต้รายการ */
    lv_obj_t *separator = lv_obj_create(parent);

    lv_obj_remove_style_all(separator);

    lv_obj_set_size(
        separator,
        315,
        1
    );

    lv_obj_set_pos(
        separator,
        82,
        y + 19
    );

    lv_obj_set_style_bg_color(
        separator,
        lv_color_hex(0x202020),
        0
    );

    lv_obj_set_style_bg_opa(
        separator,
        LV_OPA_70,
        0
    );

    /* จุดสีแดงด้านหน้ารายการ */
    lv_obj_t *dot = lv_obj_create(parent);

    lv_obj_remove_style_all(dot);

    lv_obj_set_size(
        dot,
        6,
        6
    );

    lv_obj_set_pos(
        dot,
        68,
        y + 6
    );

    lv_obj_set_style_radius(
        dot,
        LV_RADIUS_CIRCLE,
        0
    );

    lv_obj_set_style_bg_color(
        dot,
        BOOT_RED,
        0
    );

    lv_obj_set_style_bg_opa(
        dot,
        LV_OPA_COVER,
        0
    );

    /* ชื่อระบบ */
    lv_obj_t *name_label = create_label(
        parent,
        name,
        &lv_font_montserrat_14,
        BOOT_WHITE
    );

    lv_obj_set_pos(
        name_label,
        84,
        y
    );

    /* สถานะด้านขวา */
    lv_obj_t *status = create_label(
        parent,
        "WAIT",
        &lv_font_montserrat_14,
        BOOT_GREY
    );

    lv_obj_set_width(
        status,
        90
    );

    lv_obj_set_style_text_align(
        status,
        LV_TEXT_ALIGN_RIGHT,
        0
    );

    lv_obj_set_pos(
        status,
        310,
        y
    );

    return status;
}

/* ======================================================
   เปลี่ยนสถานะเป็น OK
====================================================== */

static void set_status_ok(lv_obj_t *status) {
    if (status == NULL) {
        return;
    }

    lv_label_set_text(
        status,
        "OK"
    );

    lv_obj_set_style_text_color(
        status,
        BOOT_GREEN,
        0
    );
}

/* ======================================================
   ข้อความสถานะตามเปอร์เซ็นต์
====================================================== */

static const char *get_boot_message(int value) {
    if (value < 20) {
        return "SYSTEM INITIALIZING...";
    }

    if (value < 40) {
        return "CHECKING DISPLAY...";
    }

    if (value < 60) {
        return "STARTING TOUCH DRIVER...";
    }

    if (value < 80) {
        return "LOADING UI ENGINE...";
    }

    if (value < 100) {
        return "LOADING SYSTEM PAGES...";
    }

    return "SYSTEM READY";
}

/* ======================================================
   Timer สำหรับอัปเดตหน้าบูท
====================================================== */

static void boot_timer_cb(lv_timer_t *timer) {
    /* เพิ่มเปอร์เซ็นต์ครั้งละ 2 */
    boot_value += 2;

    if (boot_value > 100) {
        boot_value = 100;
    }

    /* อัปเดต Progress Bar */
    lv_bar_set_value(
        boot_bar,
        boot_value,
        LV_ANIM_OFF
    );

    /* แปลงเปอร์เซ็นต์เป็นข้อความ */
    char percent_text[8];

    snprintf(
        percent_text,
        sizeof(percent_text),
        "%d%%",
        boot_value
    );

    /* อัปเดตข้อความบนหน้าจอ */
    lv_label_set_text(
        percent_label,
        percent_text
    );

    lv_label_set_text(
        boot_message,
        get_boot_message(boot_value)
    );

    /* เปลี่ยนสถานะระบบตามเปอร์เซ็นต์ */
    if (boot_value >= 15) {
        set_status_ok(status_display);
    }

    if (boot_value >= 32) {
        set_status_ok(status_touch);
    }

    if (boot_value >= 50) {
        set_status_ok(status_ui);
    }

    if (boot_value >= 68) {
        set_status_ok(status_memory);
    }

    if (boot_value >= 85 && boot_value < 100) {
        lv_label_set_text(
            status_pages,
            "LOADING"
        );

        lv_obj_set_style_text_color(
            status_pages,
            BOOT_YELLOW,
            0
        );
    }

    /* เมื่อบูทครบ 100% */
    if (boot_value >= 100) {
        set_status_ok(status_pages);

        /* หยุด Timer หลัก */
        lv_timer_del(timer);
        boot_timer = NULL;

        /* หน่วงให้เห็น SYSTEM READY ก่อนเข้า Desktop */
        lv_timer_create(
            [](lv_timer_t *ready_timer) {
                lv_timer_del(ready_timer);

                /* เปิดหน้า Desktop */
                desktop_create();
            },
            500,
            NULL
        );
    }
}

/* ======================================================
   สร้างหน้าบูท
====================================================== */

void boot_create() {
    /* --------------------------------------------------
       สร้าง Screen หลัก
    -------------------------------------------------- */

    boot_scr = lv_obj_create(NULL);

    lv_obj_remove_style_all(boot_scr);

    lv_obj_set_size(
        boot_scr,
        480,
        320
    );

    lv_obj_set_style_bg_color(
        boot_scr,
        BOOT_BG,
        0
    );

    lv_obj_set_style_bg_opa(
        boot_scr,
        LV_OPA_COVER,
        0
    );

    lv_obj_clear_flag(
        boot_scr,
        LV_OBJ_FLAG_SCROLLABLE
    );

    /* --------------------------------------------------
       กรอบหลักรอบหน้าจอ
    -------------------------------------------------- */

    lv_obj_t *frame = lv_obj_create(boot_scr);

    lv_obj_remove_style_all(frame);

    lv_obj_set_size(
        frame,
        476,
        316
    );

    lv_obj_center(frame);

    lv_obj_set_style_border_width(
        frame,
        1,
        0
    );

    lv_obj_set_style_border_color(
        frame,
        lv_color_hex(0x555555),
        0
    );

    lv_obj_set_style_border_opa(
        frame,
        LV_OPA_80,
        0
    );

    lv_obj_set_style_radius(
        frame,
        4,
        0
    );

    lv_obj_set_style_bg_opa(
        frame,
        LV_OPA_TRANSP,
        0
    );

    /* --------------------------------------------------
       เส้น Cyber มุมด้านบน
    -------------------------------------------------- */

    lv_obj_t *top_left = create_line(
        boot_scr,
        line_top_left,
        4,
        BOOT_RED_DARK,
        1
    );

    lv_obj_set_pos(
        top_left,
        0,
        0
    );

    lv_obj_t *top_right = create_line(
        boot_scr,
        line_top_right,
        4,
        BOOT_RED_DARK,
        1
    );

    lv_obj_set_pos(
        top_right,
        0,
        0
    );

    /* --------------------------------------------------
       เส้น Cyber มุมด้านล่าง
    -------------------------------------------------- */

    lv_obj_t *bottom_left = create_line(
        boot_scr,
        line_bottom_left,
        4,
        BOOT_RED,
        1
    );

    lv_obj_set_pos(
        bottom_left,
        0,
        0
    );

    lv_obj_t *bottom_right = create_line(
        boot_scr,
        line_bottom_right,
        4,
        BOOT_RED,
        1
    );

    lv_obj_set_pos(
        bottom_right,
        0,
        0
    );

    /* --------------------------------------------------
       Header ด้านบน
    -------------------------------------------------- */

    lv_obj_t *header_title = create_label(
        boot_scr,
        "TeYoMaRu OS",
        &lv_font_montserrat_14,
        BOOT_WHITE
    );

    lv_obj_set_pos(
        header_title,
        18,
        7
    );

    /* เวอร์ชันด้านขวา */
    lv_obj_t *version = create_label(
        boot_scr,
        "v1.0.0",
        &lv_font_montserrat_12,
        BOOT_WHITE
    );

    lv_obj_align(
        version,
        LV_ALIGN_TOP_RIGHT,
        -17,
        8
    );

    /* เส้นใต้ Header */
    lv_obj_t *header_line = lv_obj_create(boot_scr);

    lv_obj_remove_style_all(header_line);

    lv_obj_set_size(
        header_line,
        442,
        1
    );

    lv_obj_set_pos(
        header_line,
        18,
        27
    );

    lv_obj_set_style_bg_color(
        header_line,
        BOOT_RED_DARK,
        0
    );

    lv_obj_set_style_bg_opa(
        header_line,
        LV_OPA_80,
        0
    );

    /* --------------------------------------------------
       โลโก้ TeYoMaRu OS จากไฟล์ assets
    -------------------------------------------------- */

    lv_obj_t *logo_img = lv_img_create(boot_scr);

    lv_img_set_src(
        logo_img,
        &teyomaru_symbol
    );

    /*
       วางโลโก้ไว้กึ่งกลางด้านบน
       ปรับตำแหน่งแกน Y ได้จากค่าตัวสุดท้าย
    */
    lv_obj_align(
        logo_img,
        LV_ALIGN_TOP_MID,
        0,
        31
    );

    /* ป้องกันรูปภาพรับการกด */
    lv_obj_clear_flag(
        logo_img,
        LV_OBJ_FLAG_CLICKABLE
    );

    /* --------------------------------------------------
       ชื่อระบบ
    -------------------------------------------------- */

    lv_obj_t *main_title = create_label(
        boot_scr,
        "TeYoMaRu OS",
        &lv_font_montserrat_24,
        BOOT_WHITE
    );

    lv_obj_align(
        main_title,
        LV_ALIGN_TOP_MID,
        0,
        82
    );

    /*
       LVGL 8.4 ไม่มีคำสั่ง:
       lv_obj_set_style_text_outline_color()
       lv_obj_set_style_text_outline_width()

       จึงไม่ใช้ Text Outline ในส่วนนี้
    */

    /* Subtitle */
    lv_obj_t *subtitle = create_label(
        boot_scr,
        "PERSONAL LAUNCHER",
        &lv_font_montserrat_12,
        BOOT_RED
    );

    lv_obj_align(
        subtitle,
        LV_ALIGN_TOP_MID,
        0,
        110
    );

    /* --------------------------------------------------
       ข้อความสถานะหลัก
    -------------------------------------------------- */

    boot_message = create_label(
        boot_scr,
        "SYSTEM INITIALIZING...",
        &lv_font_montserrat_14,
        BOOT_RED
    );

    lv_obj_align(
        boot_message,
        LV_ALIGN_TOP_MID,
        0,
        132
    );

    /* เส้นซ้ายของข้อความสถานะ */
    lv_obj_t *message_line_left =
        lv_obj_create(boot_scr);

    lv_obj_remove_style_all(
        message_line_left
    );

    lv_obj_set_size(
        message_line_left,
        70,
        1
    );

    lv_obj_set_pos(
        message_line_left,
        40,
        142
    );

    lv_obj_set_style_bg_color(
        message_line_left,
        BOOT_RED_DARK,
        0
    );

    lv_obj_set_style_bg_opa(
        message_line_left,
        LV_OPA_COVER,
        0
    );

    /* เส้นขวาของข้อความสถานะ */
    lv_obj_t *message_line_right =
        lv_obj_create(boot_scr);

    lv_obj_remove_style_all(
        message_line_right
    );

    lv_obj_set_size(
        message_line_right,
        70,
        1
    );

    lv_obj_set_pos(
        message_line_right,
        370,
        142
    );

    lv_obj_set_style_bg_color(
        message_line_right,
        BOOT_RED_DARK,
        0
    );

    lv_obj_set_style_bg_opa(
        message_line_right,
        LV_OPA_COVER,
        0
    );

    /* --------------------------------------------------
       รายการสถานะระบบ
    -------------------------------------------------- */

    status_display = create_status_row(
        boot_scr,
        154,
        "DISPLAY"
    );

    status_touch = create_status_row(
        boot_scr,
        175,
        "TOUCH"
    );

    status_ui = create_status_row(
        boot_scr,
        196,
        "UI ENGINE"
    );

    status_memory = create_status_row(
        boot_scr,
        217,
        "MEMORY"
    );

    status_pages = create_status_row(
        boot_scr,
        238,
        "PAGES"
    );

    /* --------------------------------------------------
       ตัวเลขเปอร์เซ็นต์
    -------------------------------------------------- */

    percent_label = create_label(
        boot_scr,
        "0%",
        &lv_font_montserrat_20,
        BOOT_RED
    );

    lv_obj_set_width(
        percent_label,
        70
    );

    lv_obj_set_style_text_align(
        percent_label,
        LV_TEXT_ALIGN_RIGHT,
        0
    );

    lv_obj_set_pos(
        percent_label,
        20,
        269
    );

    /* --------------------------------------------------
       Progress Bar
    -------------------------------------------------- */

    boot_bar = lv_bar_create(boot_scr);

    lv_obj_set_size(
        boot_bar,
        330,
        20
    );

    lv_obj_set_pos(
        boot_bar,
        102,
        270
    );

    lv_bar_set_range(
        boot_bar,
        0,
        100
    );

    lv_bar_set_value(
        boot_bar,
        0,
        LV_ANIM_OFF
    );

    /* พื้นหลังของ Progress Bar */
    lv_obj_set_style_bg_color(
        boot_bar,
        BOOT_PANEL,
        LV_PART_MAIN
    );

    lv_obj_set_style_bg_opa(
        boot_bar,
        LV_OPA_COVER,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_color(
        boot_bar,
        BOOT_RED_DARK,
        LV_PART_MAIN
    );

    lv_obj_set_style_border_width(
        boot_bar,
        1,
        LV_PART_MAIN
    );

    lv_obj_set_style_radius(
        boot_bar,
        5,
        LV_PART_MAIN
    );

    lv_obj_set_style_pad_all(
        boot_bar,
        3,
        LV_PART_MAIN
    );

    /* ส่วน Progress ที่โหลดแล้ว */
    lv_obj_set_style_bg_color(
        boot_bar,
        BOOT_RED,
        LV_PART_INDICATOR
    );

    lv_obj_set_style_bg_opa(
        boot_bar,
        LV_OPA_COVER,
        LV_PART_INDICATOR
    );

    lv_obj_set_style_radius(
        boot_bar,
        2,
        LV_PART_INDICATOR
    );

    /* --------------------------------------------------
       ข้อความด้านล่าง
    -------------------------------------------------- */

    lv_obj_t *bottom_text = create_label(
        boot_scr,
        ">>  BOOTING TEYOMARU OS  <<",
        &lv_font_montserrat_12,
        BOOT_GREY
    );

    lv_obj_align(
        bottom_text,
        LV_ALIGN_BOTTOM_MID,
        0,
        -7
    );
}

/* ======================================================
   เริ่มหน้าบูท
====================================================== */

void boot_start() {
    /* Reset ค่า Progress */
    boot_value = 0;

    /* ถ้ายังไม่ได้สร้างหน้าบูท ให้สร้างก่อน */
    if (boot_scr == NULL) {
        boot_create();
    }

    /* โหลดหน้าบูทขึ้นหน้าจอ */
    lv_scr_load(boot_scr);

    /* Reset Progress Bar */
    lv_bar_set_value(
        boot_bar,
        0,
        LV_ANIM_OFF
    );

    /* Reset ตัวเลขเปอร์เซ็นต์ */
    lv_label_set_text(
        percent_label,
        "0%"
    );

    /* Reset ข้อความสถานะ */
    lv_label_set_text(
        boot_message,
        "SYSTEM INITIALIZING..."
    );

    /* Reset สถานะระบบทั้งหมด */
    lv_label_set_text(
        status_display,
        "WAIT"
    );

    lv_label_set_text(
        status_touch,
        "WAIT"
    );

    lv_label_set_text(
        status_ui,
        "WAIT"
    );

    lv_label_set_text(
        status_memory,
        "WAIT"
    );

    lv_label_set_text(
        status_pages,
        "WAIT"
    );

    /* Reset สีสถานะกลับเป็นสีเทา */
    lv_obj_set_style_text_color(
        status_display,
        BOOT_GREY,
        0
    );

    lv_obj_set_style_text_color(
        status_touch,
        BOOT_GREY,
        0
    );

    lv_obj_set_style_text_color(
        status_ui,
        BOOT_GREY,
        0
    );

    lv_obj_set_style_text_color(
        status_memory,
        BOOT_GREY,
        0
    );

    lv_obj_set_style_text_color(
        status_pages,
        BOOT_GREY,
        0
    );

    /* ป้องกัน Timer ซ้ำ */
    if (boot_timer != NULL) {
        lv_timer_del(boot_timer);
        boot_timer = NULL;
    }

    /* เริ่ม Timer อัปเดตทุก 60 ms */
    boot_timer = lv_timer_create(
        boot_timer_cb,
        60,
        NULL
    );
}