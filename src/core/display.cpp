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

   ใช้ Buffer สูง 10 แถว
   รองรับทั้งแนวนอน 480x320 และแนวตั้ง 320x480
====================================================== */

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_WIDTH * 10];

/* Driver หลักของ LVGL */
static lv_disp_drv_t disp_drv;

/* ใช้เช็กว่า Register Driver แล้วหรือยัง */
static bool disp_registered = false;

/* false = Landscape
   true  = Portrait */
static bool portrait_mode = false;

/* ======================================================
   LVGL Flush Callback

   ทำหน้าที่ส่งข้อมูลภาพจาก LVGL ไปยังจอ TFT

   หมายเหตุ:
   - ใช้ pushColors เพื่อส่งข้อมูลรวดเดียว
   - true คือให้ TFT_eSPI จัดการ byte swap
====================================================== */

static void my_disp_flush(
  lv_disp_drv_t *disp,
  const lv_area_t *area,
  lv_color_t *color_p
) {
  /* คำนวณความกว้างและความสูงของพื้นที่ */
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;

  /* จำนวน Pixel ทั้งหมด */
  uint32_t pixel_count = w * h;

  /* เริ่มเขียนข้อมูลไปยังจอ */
  tft.startWrite();

  /* กำหนดพื้นที่ที่จะวาด */
  tft.setAddrWindow(
    area->x1,
    area->y1,
    w,
    h
  );

  /* ส่งข้อมูลสีจาก LVGL ไปยัง TFT */
  tft.pushColors(
    reinterpret_cast<uint16_t *>(color_p),
    pixel_count,
    true
  );

  /* จบการเขียน */
  tft.endWrite();

  /* แจ้ง LVGL ว่าวาดเสร็จแล้ว */
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

  /* ถ้า Register LVGL Driver แล้ว
     ให้อัปเดตความละเอียดใหม่ */
  if (disp_registered) {
    lv_disp_drv_update(
      lv_disp_get_default(),
      &disp_drv
    );

    /* สั่งให้ LVGL วาดหน้าใหม่ทั้งหมด */
    lv_obj_invalidate(
      lv_scr_act()
    );
  }

  /* ล้างจอเป็นสีดำ */
  tft.fillScreen(TFT_BLACK);
}

/* ======================================================
   เริ่มต้นระบบจอ
====================================================== */

void display_init() {
  /* เปิดไฟ Backlight */
  display_backlight_on();

  /* เริ่มต้น TFT */
  tft.begin();

  /* ตั้งค่าเริ่มต้นเป็นแนวนอน 480x320 */
  tft.setRotation(1);

  /*
     สำหรับจอ ILI9488 ชุดนี้
     ต้องใช้ invertDisplay(true)
     เพื่อให้สีดำ/ขาวและสีอื่นแสดงถูกต้อง
  */
  tft.invertDisplay(true);

  /* ล้างหน้าจอเป็นสีดำก่อนเริ่ม LVGL */
  tft.fillScreen(TFT_BLACK);

  /* เริ่มต้น LVGL */
  lv_init();

  /* สร้าง Draw Buffer */
  lv_disp_draw_buf_init(
    &draw_buf,
    buf1,
    NULL,
    SCREEN_WIDTH * 10
  );

  /* เริ่มต้น Display Driver */
  lv_disp_drv_init(&disp_drv);

  /* กำหนดความละเอียดเริ่มต้น */
  disp_drv.hor_res = SCREEN_WIDTH;
  disp_drv.ver_res = SCREEN_HEIGHT;

  /* กำหนด Callback สำหรับวาดจอ */
  disp_drv.flush_cb = my_disp_flush;

  /* เชื่อม Draw Buffer เข้ากับ Driver */
  disp_drv.draw_buf = &draw_buf;

  /* Register Display Driver */
  lv_disp_drv_register(&disp_drv);

  /* บันทึกสถานะว่า Register แล้ว */
  disp_registered = true;

  /* ค่าเริ่มต้นเป็น Landscape */
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
   ตรวจสอบว่าปัจจุบันเป็นแนวตั้งหรือไม่
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