#include "display.h"
#include <TFT_eSPI.h>

/* ======================================================
   Display Driver
   - รองรับจอ ILI9488
   - Landscape 480x320
   - Portrait 320x480
   - ใช้ LVGL 8.x
====================================================== */

static TFT_eSPI tft = TFT_eSPI();

/* ======================================================
   LVGL Draw Buffer

   เดิมใช้เพียง 6 แถว ทำให้การเลื่อนกระตุก
   ปรับเป็น 30 แถว เพื่อให้ส่งภาพต่อครั้งได้มากขึ้น

   480 × 30 × 2 bytes ≈ 28.8 KB
====================================================== */

#define DISPLAY_BUFFER_LINES 8

static lv_disp_draw_buf_t draw_buf;

static lv_color_t buf1[
    SCREEN_WIDTH * DISPLAY_BUFFER_LINES
];

/* Driver หลักของ LVGL */
static lv_disp_drv_t disp_drv;

/* ใช้เช็กว่า Register Driver แล้วหรือยัง */
static bool disp_registered = false;

/* false = Landscape
   true  = Portrait */
static bool portrait_mode = false;

/* ======================================================
   LVGL Flush Callback

   ส่งข้อมูลภาพจาก LVGL ไปยังจอ TFT
====================================================== */

static void my_disp_flush(
    lv_disp_drv_t *disp,
    const lv_area_t *area,
    lv_color_t *color_p
) {
    /* คำนวณขนาดพื้นที่ */
    uint32_t width =
        area->x2 - area->x1 + 1;

    uint32_t height =
        area->y2 - area->y1 + 1;

    uint32_t pixel_count =
        width * height;

    /* เริ่มส่งข้อมูลไปยังจอ */
    tft.startWrite();

    tft.setAddrWindow(
        area->x1,
        area->y1,
        width,
        height
    );

    /*
       true = ให้ TFT_eSPI สลับ Byte
       ให้ตรงกับข้อมูลสี RGB565 ของ LVGL
    */
    tft.pushColors(
        reinterpret_cast<uint16_t *>(color_p),
        pixel_count,
        true
    );

    tft.endWrite();

    /* แจ้ง LVGL ว่าส่งภาพเสร็จแล้ว */
    lv_disp_flush_ready(disp);
}

/* ======================================================
   เปลี่ยนแนวหน้าจอ

   portrait = true  -> 320x480
   portrait = false -> 480x320
====================================================== */

static void apply_orientation(bool portrait) {
    portrait_mode = portrait;

    if (portrait_mode) {
        /* แนวตั้ง */
        tft.setRotation(0);

        disp_drv.hor_res = 320;
        disp_drv.ver_res = 480;
    } else {
        /* แนวนอน */
        tft.setRotation(1);

        disp_drv.hor_res = 480;
        disp_drv.ver_res = 320;
    }

    /*
       ถ้า Register Driver แล้ว
       ให้อัปเดตความละเอียด
    */
    if (disp_registered) {
        lv_disp_t *display =
            lv_disp_get_default();

        if (display != nullptr) {
            lv_disp_drv_update(
                display,
                &disp_drv
            );
        }

        /* สั่งวาดหน้าปัจจุบันใหม่ */
        lv_obj_t *screen =
            lv_scr_act();

        if (screen != nullptr) {
            lv_obj_invalidate(screen);
        }
    }

    /* ล้างจอหลังเปลี่ยนแนว */
    tft.fillScreen(TFT_BLACK);
}

/* ======================================================
   เริ่มต้นระบบจอ
====================================================== */

void display_init() {
    /* เปิด Backlight */
    display_backlight_on();

    /* เริ่มต้น TFT */
    tft.begin();

    /* แนวนอน 480x320 */
    tft.setRotation(1);

    /*
       จอ ILI9488 ชุดนี้ต้องกลับสี
       เพื่อให้สีดำและสีอื่นแสดงถูกต้อง
    */
    tft.invertDisplay(true);

    /* ล้างจอก่อนเริ่ม LVGL */
    tft.fillScreen(TFT_BLACK);

    /* เริ่มต้น LVGL */
    lv_init();

    /* สร้าง Draw Buffer ขนาด 30 แถว */
    lv_disp_draw_buf_init(
        &draw_buf,
        buf1,
        nullptr,
        SCREEN_WIDTH * DISPLAY_BUFFER_LINES
    );

    /* เริ่มต้น Display Driver */
    lv_disp_drv_init(&disp_drv);

    /* ความละเอียดเริ่มต้น */
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;

    /* Callback สำหรับส่งภาพ */
    disp_drv.flush_cb = my_disp_flush;

    /* เชื่อม Buffer กับ Driver */
    disp_drv.draw_buf = &draw_buf;

    /* Register Driver */
    lv_disp_drv_register(&disp_drv);

    disp_registered = true;
    portrait_mode = false;
}

/* ======================================================
   เปลี่ยนเป็นแนวนอน 480x320
====================================================== */

void display_set_landscape() {
    apply_orientation(false);
}

/* ======================================================
   เปลี่ยนเป็นแนวตั้ง 320x480
====================================================== */

void display_set_portrait() {
    apply_orientation(true);
}

/* ======================================================
   ตรวจสอบแนวหน้าจอ
====================================================== */

bool display_is_portrait() {
    return portrait_mode;
}

/* ======================================================
   เปิด Backlight
====================================================== */

void display_backlight_on() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
}

/* ======================================================
   ปิด Backlight
====================================================== */

void display_backlight_off() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);
}