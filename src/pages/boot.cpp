#include "boot.h"
#include "desktop.h"
#include "../core/theme.h"
#include "../assets/teyomaru_logo.h"
#include "../assets/fonts/teyomaru_fonts.h"


#include <lvgl.h>
#include <cstdio>

/* ======================================================
   คู่มือปรับตำแหน่งและขนาดบนหน้าบูท
   ======================================================

   หลักการสำคัญของ LVGL ที่ใช้ในไฟล์นี้

   1) lv_obj_set_pos(object, x, y);
      - x = ระยะจากด้านซ้ายของจอ
      - y = ระยะจากด้านบนของจอ
      - x มากขึ้น  -> ขยับไปทางขวา
      - x น้อยลง   -> ขยับไปทางซ้าย
      - y มากขึ้น  -> ขยับลง
      - y น้อยลง   -> ขยับขึ้น

   2) lv_obj_align(object, จุดอ้างอิง, x_offset, y_offset);
      ตัวอย่าง:
      lv_obj_align(main_title, LV_ALIGN_TOP_MID, 0, 95);

      - LV_ALIGN_TOP_MID = อ้างอิงกึ่งกลางด้านบนของจอ
      - ค่า 0            = ไม่ขยับซ้าย/ขวา
      - ค่า 95           = ระยะจากด้านบนของจอ

      หากต้องการขยับข้อความขึ้น:
      95 -> 85

      หากต้องการขยับข้อความลง:
      95 -> 105

   3) lv_img_set_zoom(image, zoom);
      ใช้ปรับขนาดรูปภาพ

      - 256 = 100%
      - 192 = 75%
      - 160 = 62.5%
      - 128 = 50%

      ตัวเลขมากขึ้น = รูปใหญ่ขึ้น
      ตัวเลขน้อยลง  = รูปเล็กลง

   4) การเปลี่ยนขนาดตัวอักษร
      เปลี่ยนชื่อ Font ที่ส่งให้ create_label()

      ตัวอย่าง:
      &font_teyomaru_20  = ตัวใหญ่
      &font_teyomaru_10  = ตัวเล็ก

   จุดที่แก้บ่อยในไฟล์นี้

   - โลโก้:
     ค้นหา "lv_img_set_zoom" และ "logo_img"

   - คำว่า TeYoMaRu OS:
     ค้นหา "main_title"

   - คำว่า PERSONAL LAUNCHER:
     ค้นหา "sub_title"

   - ข้อความสีแดงสถานะระบบ:
     ค้นหา "boot_message"

   - รายการ DISPLAY / TOUCH / UI ENGINE:
     ค้นหา "status_display"

   - ตัวเลขเปอร์เซ็นต์:
     ค้นหา "percent_label"

   - แถบโหลด:
     ค้นหา "boot_bar"
====================================================== */

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
        &font_teyomaru_10,
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
        &font_teyomaru_10,
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
        &font_teyomaru_10,
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
        &font_teyomaru_10,
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
       ============================
       ปรับขนาดโลโก้ตรงนี้
       ============================

       ค่า Zoom ของ LVGL:
       256 = 100% ขนาดเดิม
       192 = 75%
       160 = 62.5%
       130 = ประมาณ 51%
       128 = 50%

       วิธีแก้:
       - โลโก้ใหญ่เกินไป  -> ลดเลข 130 ลง เช่น 115
       - โลโก้เล็กเกินไป  -> เพิ่มเลข 130 ขึ้น เช่น 145
    */
    lv_img_set_zoom(
        logo_img,
        130
    );

    /*
       ============================
       ปรับตำแหน่งโลโก้ตรงนี้
       ============================

       LV_ALIGN_TOP_MID = ยึดกึ่งกลางด้านบนของจอ
       ค่า 0  = ตำแหน่งซ้าย/ขวา
       ค่า 20 = ตำแหน่งขึ้น/ลง

       วิธีแก้:
       - ขยับโลโก้ขึ้น   -> ลด 20 เช่น 10
       - ขยับโลโก้ลง    -> เพิ่ม 20 เช่น 30
       - ขยับไปทางซ้าย  -> เปลี่ยน 0 เป็นค่าติดลบ เช่น -10
       - ขยับไปทางขวา   -> เปลี่ยน 0 เป็นค่าบวก เช่น 10
    */
    lv_obj_align(
        logo_img,
        LV_ALIGN_TOP_MID,
        0,
        20
    );

    /* ป้องกันรูปภาพรับการกด */
    lv_obj_clear_flag(
        logo_img,
        LV_OBJ_FLAG_CLICKABLE
    );

    /* --------------------------------------------------
       ชื่อระบบใต้โลโก้
    -------------------------------------------------- */

    /*
       ============================
       ชื่อหลัก: TeYoMaRu OS
       ============================

       จุดแก้ขนาดตัวอักษร:
       &font_teyomaru_20

       วิธีแก้:
       - ตัวใหญ่เกินไป -> เปลี่ยนเป็น &font_teyomaru_14
       - ต้องการตัวใหญ่ -> ใช้ &font_teyomaru_20

       จุดแก้ตำแหน่ง:
       lv_obj_align(..., 0, 95);

       - 95 น้อยลง = ขยับขึ้น
       - 95 มากขึ้น = ขยับลง
       - ค่า 0 เปลี่ยนเป็นค่าติดลบ = ขยับซ้าย
       - ค่า 0 เปลี่ยนเป็นค่าบวก  = ขยับขวา
    */
    lv_obj_t *main_title = create_label(
        boot_scr,
        "TeYoMaRu OS",
        &font_teyomaru_20,
        BOOT_WHITE
    );

    lv_obj_align(
        main_title,
        LV_ALIGN_TOP_MID,
        0,
        105
    );

    /*
       ============================
       ชื่อรอง: PERSONAL LAUNCHER
       ============================

       ขนาดตัวอักษร:
       &font_teyomaru_10

       ตำแหน่ง:
       lv_obj_align(..., 0, 121);

       วิธีแก้:
       - ขยับขึ้น -> ลด 121 เช่น 112
       - ขยับลง  -> เพิ่ม 121 เช่น 130

       ระยะห่างจาก TeYoMaRu OS:
       main_title อยู่ที่ 95
       sub_title  อยู่ที่ 121
       ระยะห่างประมาณ 26 px

       หากข้อความซ้อนกัน:
       - เพิ่มค่า sub_title เช่น 126
       หรือ
       - ลดค่า main_title เช่น 90
    */
    lv_obj_t *sub_title = create_label(
        boot_scr,
        "PERSONAL LAUNCHER",
        &font_teyomaru_10,
        BOOT_RED
    );

    lv_obj_align(
        sub_title,
        LV_ALIGN_TOP_MID,
        0,
        131
    );

    /* --------------------------------------------------
       ข้อความสถานะหลัก
       ย้ายลงไปไว้ใต้ Progress Bar
    -------------------------------------------------- */

    /*
       ============================
       ข้อความสถานะสีแดงด้านล่าง
       ============================

       ข้อความนี้จะเปลี่ยนอัตโนมัติตามเปอร์เซ็นต์
       เช่น:
       SYSTEM INITIALIZING...
       CHECKING DISPLAY...
       STARTING TOUCH DRIVER...
       LOADING UI ENGINE...
       LOADING SYSTEM PAGES...
       SYSTEM READY

       ตำแหน่งปัจจุบัน:
       y = 279

       วิธีแก้:
       - ขยับขึ้น -> ลด 279 เช่น 270
       - ขยับลง  -> เพิ่ม 279 เช่น 288

       ระวัง:
       จอสูง 320 px
       หากใช้ค่าเกินประมาณ 295 อาจตกขอบ
    */
    boot_message = create_label(
        boot_scr,
        "SYSTEM INITIALIZING...",
        &font_teyomaru_10,
        BOOT_RED
    );

    lv_obj_align(
        boot_message,
        LV_ALIGN_TOP_MID,
        0,
        319
    );

    /* --------------------------------------------------
       รายการสถานะระบบ
    -------------------------------------------------- */

    /*
       ============================
       ตำแหน่งรายการสถานะระบบ
       ============================

       ตัวเลขตัวที่ 2 คือแกน Y ของแต่ละแถว

       DISPLAY   = 122
       TOUCH     = 142
       UI ENGINE = 162
       MEMORY    = 182
       PAGES     = 202

       วิธีขยับทั้งชุด:
       - ขยับขึ้น 10 px -> ลบทุกค่าด้วย 10
         122,142,162,182,202
         เปลี่ยนเป็น
         112,132,152,172,192

       - ขยับลง 10 px -> บวกทุกค่าด้วย 10
    */
    status_display = create_status_row(
        boot_scr,
        152,
        "DISPLAY"
    );

    status_touch = create_status_row(
        boot_scr,
        172,
        "TOUCH"
    );

    status_ui = create_status_row(
        boot_scr,
        192,
        "UI ENGINE"
    );

    status_memory = create_status_row(
        boot_scr,
        212,
        "MEMORY"
    );

    status_pages = create_status_row(
        boot_scr,
        232,
        "PAGES"
    );

    /* --------------------------------------------------
       ตัวเลขเปอร์เซ็นต์
    -------------------------------------------------- */

    percent_label = create_label(
        boot_scr,
        "0%",
        &font_teyomaru_20,
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

    /*
       ============================
       ตำแหน่งตัวเลขเปอร์เซ็นต์
       ============================

       x = 20  ระยะจากซ้าย
       y = 232 ระยะจากบน

       วิธีแก้:
       - ขยับขวา -> เพิ่ม 20
       - ขยับซ้าย -> ลด 20
       - ขยับลง -> เพิ่ม 232
       - ขยับขึ้น -> ลด 232
    */
    lv_obj_set_pos(
        percent_label,
        20,
        272
    );

    /* --------------------------------------------------
       Progress Bar
    -------------------------------------------------- */

    boot_bar = lv_bar_create(boot_scr);

    /*
       ============================
       ขนาดและตำแหน่ง Progress Bar
       ============================

       lv_obj_set_size(boot_bar, 330, 20);
       - 330 = ความกว้าง
       - 20  = ความสูง

       วิธีแก้ขนาด:
       - ต้องการแถบยาวขึ้น -> เพิ่ม 330
       - ต้องการแถบสั้นลง -> ลด 330
       - ต้องการแถบหนาขึ้น -> เพิ่ม 20
       - ต้องการแถบบางลง -> ลด 20

       lv_obj_set_pos(boot_bar, 102, 234);
       - 102 = ระยะจากซ้าย
       - 234 = ระยะจากบน

       วิธีแก้ตำแหน่ง:
       - ขยับขวา -> เพิ่ม 102
       - ขยับซ้าย -> ลด 102
       - ขยับลง -> เพิ่ม 234
       - ขยับขึ้น -> ลด 234
    */
    lv_obj_set_size(
        boot_bar,
        330,
        20
    );

    lv_obj_set_pos(
        boot_bar,
        102,
        274
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

    /* ข้อความด้านล่างใช้ boot_message แบบไดนามิกแทน */
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