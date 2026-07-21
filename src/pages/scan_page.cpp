#include "scan_page.h"
#include "desktop.h"
#include "../core/theme.h"
#include "../core/display.h"
#include "../widgets/statusbar.h"

#include <lvgl.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <esp_heap_caps.h>
#include <qrcodegen.h>

/* ======================================================
   คู่มือปรับหน้า Scan & Pay
   ======================================================

   ไฟล์นี้มี 2 หน้าหลัก

   1) หน้ากรอกจำนวนเงินแนวนอน 480 x 320
      - แสดงจำนวนเงิน
      - ปุ่มตัวเลข
      - ปุ่ม CREATE QR
      - ปุ่ม BACK

   2) หน้าแสดง QR แนวตั้ง 320 x 480
      - แสดงจำนวนเงินด้านบน
      - QR Code ตรงกลาง
      - ปุ่ม BACK ด้านล่าง

   หลักการปรับตำแหน่งใน LVGL

   lv_obj_set_pos(object, x, y);
   - x มากขึ้น = ขยับไปทางขวา
   - x น้อยลง = ขยับไปทางซ้าย
   - y มากขึ้น = ขยับลง
   - y น้อยลง = ขยับขึ้น

   lv_obj_set_size(object, width, height);
   - width  = ความกว้าง
   - height = ความสูง

   lv_obj_align(object, จุดอ้างอิง, x_offset, y_offset);
   ตัวอย่าง:
   /*
     ตำแหน่งจำนวนเงินในหน้า QR

     LV_ALIGN_TOP_MID = กึ่งกลางด้านบน
     x = 0
     y = 24

     ขยับขึ้น -> ลด 24
     ขยับลง  -> เพิ่ม 24
  */
  lv_obj_align(amount, LV_ALIGN_TOP_MID, 0, 24);

   - ค่า x_offset ติดลบ = ขยับซ้าย
   - ค่า x_offset เป็นบวก = ขยับขวา
   - ค่า y_offset ติดลบ = ขยับขึ้น
   - ค่า y_offset เป็นบวก = ขยับลง

   จุดที่แก้บ่อย

   - กล่องจำนวนเงิน:
     ค้นหา amount_box

   - ตัวเลขจำนวนเงิน:
     ค้นหา amount_label

   - ชื่อ Scan & Pay:
     ค้นหา title

   - แป้นตัวเลข:
     ค้นหา btn_w, btn_h, gap_x, gap_y, start_x, start_y

   - ปุ่ม CREATE QR และ BACK:
     ค้นหา action_x, action_w, action_h

   - ขนาด QR:
     ค้นหา QR_CANVAS_SIZE

   - ตำแหน่ง QR:
     ค้นหา qr_box

   - ปุ่ม Back หน้า QR:
     ค้นหา back_button ใน show_qr_portrait()
====================================================== */

/* ======================================================
   PromptPay ID

   ใส่ได้ 2 แบบ:
   1) เบอร์โทรศัพท์ 10 หลัก เช่น 0812345678
   2) เลขบัตรประชาชน 13 หลัก

   สำคัญ:
   เปลี่ยนค่าในเครื่องหมายคำพูดเป็นข้อมูลจริงก่อนใช้งาน
====================================================== */
#define PROMPTPAY_ID "0812345678"

/* ======================================================
   ขนาดหน้าจอ

   LANDSCAPE = แนวนอน
   PORTRAIT  = แนวตั้ง

   ไม่ควรเปลี่ยน หากใช้จอ 480 x 320
====================================================== */
#define LANDSCAPE_W 480
#define LANDSCAPE_H 320
#define PORTRAIT_W  320
#define PORTRAIT_H  480

/* ======================================================
   Buffer เก็บจำนวนเงิน

   AMOUNT_MAX = จำนวนหลักสูงสุดที่กรอกได้
   10 หมายถึงกรอกตัวเลขได้สูงสุด 10 หลัก

   amount_buf = เก็บตัวเลขที่ผู้ใช้กด
   amount_len = จำนวนหลักที่กรอกแล้ว
====================================================== */
#define AMOUNT_MAX 10
static char amount_buf[AMOUNT_MAX + 1] = {0};
static int amount_len = 0;

static lv_obj_t *scr_input = NULL;
static lv_obj_t *amount_label = NULL;
static lv_obj_t *scr_qr = NULL;

/* ======================================================
   ขนาด QR Code

   QR_CANVAS_SIZE = ขนาดพื้นที่ QR เป็นพิกเซล
   - เพิ่มค่า = QR ใหญ่ขึ้น
   - ลดค่า   = QR เล็กลง

   ตัวอย่าง:
   220 = ขนาดปัจจุบัน
   200 = เล็กลง
   240 = ใหญ่ขึ้น

   QR_QUIET_ZONE = ขอบขาวรอบ QR
   ปกติควรใช้ 4 เพื่อให้สแกนง่าย
====================================================== */
#define QR_CANVAS_SIZE 220
#define QR_QUIET_ZONE 4

static lv_color_t *qr_buf = NULL;
static uint8_t qr_temp_buffer[qrcodegen_BUFFER_LEN_MAX];
static uint8_t qr_code_buffer[qrcodegen_BUFFER_LEN_MAX];

static void scan_page_create_input();
static void show_qr_portrait();
static void refresh_amount_label();

static uint16_t crc16_ccitt(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;

    for (int bit = 0; bit < 8; bit++) {
      crc = (crc & 0x8000)
        ? (uint16_t)((crc << 1) ^ 0x1021)
        : (uint16_t)(crc << 1);
    }
  }

  return crc;
}

static bool build_promptpay_payload(
  char *out,
  size_t out_size,
  const char *amount_str
) {
  if (!out || !amount_str || out_size == 0) return false;

  const size_t id_len = strlen(PROMPTPAY_ID);
  char proxy_value[20] = {0};
  const char *proxy_tag = NULL;

  if (id_len == 10 && PROMPTPAY_ID[0] == '0') {
    // 0812345678 -> 0066812345678
    snprintf(proxy_value, sizeof(proxy_value), "0066%s", PROMPTPAY_ID + 1);
    proxy_tag = "01";
  } else if (id_len == 13) {
    snprintf(proxy_value, sizeof(proxy_value), "%s", PROMPTPAY_ID);
    proxy_tag = "02";
  } else {
    out[0] = '\0';
    return false;
  }

  char merchant_account[96];
  snprintf(
    merchant_account,
    sizeof(merchant_account),
    "0016A000000677010111%s%02u%s",
    proxy_tag,
    (unsigned)strlen(proxy_value),
    proxy_value
  );

  char tag29[112];
  snprintf(
    tag29,
    sizeof(tag29),
    "29%02u%s",
    (unsigned)strlen(merchant_account),
    merchant_account
  );

  char tag54[32];
  snprintf(
    tag54,
    sizeof(tag54),
    "54%02u%s",
    (unsigned)strlen(amount_str),
    amount_str
  );

  char body[320];
  int written = snprintf(
    body,
    sizeof(body),
    "000201"
    "010212"
    "%s"
    "5303764"
    "%s"
    "5802TH"
    "6304",
    tag29,
    tag54
  );

  if (written <= 0 || (size_t)written >= sizeof(body)) {
    out[0] = '\0';
    return false;
  }

  const uint16_t crc = crc16_ccitt(
    (const uint8_t *)body,
    strlen(body)
  );

  written = snprintf(out, out_size, "%s%04X", body, crc);
  return written > 0 && (size_t)written < out_size;
}

static void free_qr_buffer() {
  if (qr_buf) {
    heap_caps_free(qr_buf);
    qr_buf = NULL;
  }
}

static bool draw_qr_canvas(lv_obj_t *parent, const char *payload) {
  if (!parent || !payload || payload[0] == '\0') return false;

  const bool ok = qrcodegen_encodeText(
    payload,
    qr_temp_buffer,
    qr_code_buffer,
    qrcodegen_Ecc_MEDIUM,
    qrcodegen_VERSION_MIN,
    qrcodegen_VERSION_MAX,
    qrcodegen_Mask_AUTO,
    true
  );

  if (!ok) return false;

  const int qr_size = qrcodegen_getSize(qr_code_buffer);
  const int total_modules = qr_size + (QR_QUIET_ZONE * 2);
  const int module_px = QR_CANVAS_SIZE / total_modules;

  if (module_px < 1) return false;

  const int draw_size = total_modules * module_px;
  const int offset = (QR_CANVAS_SIZE - draw_size) / 2;

  free_qr_buffer();

  qr_buf = (lv_color_t *)heap_caps_malloc(
    QR_CANVAS_SIZE * QR_CANVAS_SIZE * sizeof(lv_color_t),
    MALLOC_CAP_8BIT
  );

  if (!qr_buf) return false;

  lv_obj_t *canvas = lv_canvas_create(parent);
  lv_canvas_set_buffer(
    canvas,
    qr_buf,
    QR_CANVAS_SIZE,
    QR_CANVAS_SIZE,
    LV_IMG_CF_TRUE_COLOR
  );
  lv_obj_center(canvas);
  lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER);

  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);
  rect_dsc.bg_color = lv_color_black();
  rect_dsc.bg_opa = LV_OPA_COVER;
  rect_dsc.border_width = 0;
  rect_dsc.radius = 0;

  for (int y = 0; y < qr_size; y++) {
    for (int x = 0; x < qr_size; x++) {
      if (!qrcodegen_getModule(qr_code_buffer, x, y)) continue;

      const int px = offset + (x + QR_QUIET_ZONE) * module_px;
      const int py = offset + (y + QR_QUIET_ZONE) * module_px;

      lv_canvas_draw_rect(
        canvas,
        px,
        py,
        module_px,
        module_px,
        &rect_dsc
      );
    }
  }

  return true;
}

static void refresh_amount_label() {
  if (!amount_label) return;

  if (amount_len == 0) {
    lv_label_set_text(amount_label, "0.00 THB");
    return;
  }

  const int satang = atoi(amount_buf);
  char text[32];
  snprintf(text, sizeof(text), "%d.%02d THB", satang / 100, satang % 100);
  lv_label_set_text(amount_label, text);
}

static void numpad_cb(lv_event_t *event) {
  const char *digit = (const char *)lv_event_get_user_data(event);
  if (!digit || amount_len >= AMOUNT_MAX) return;

  amount_buf[amount_len++] = digit[0];
  amount_buf[amount_len] = '\0';
  refresh_amount_label();
}

static void backspace_cb(lv_event_t *event) {
  (void)event;

  if (amount_len > 0) {
    amount_buf[--amount_len] = '\0';
    refresh_amount_label();
  }
}

static void leave_scan_page_cb(lv_event_t *event) {
  (void)event;

  free_qr_buffer();
  amount_len = 0;
  amount_buf[0] = '\0';
  display_set_landscape();
  desktop_create();
}

static void qr_back_cb(lv_event_t *event) {
  (void)event;

  free_qr_buffer();
  display_set_landscape();

  // กลับหน้ากรอกจำนวน โดยยังเก็บจำนวนเดิมไว้
  scan_page_create_input();
}

static void create_qr_cb(lv_event_t *event) {
  (void)event;

  if (amount_len == 0 || atoi(amount_buf) <= 0) return;
  show_qr_portrait();
}

static void add_button_label(
  lv_obj_t *button,
  const char *text,
  const lv_font_t *font
) {
  lv_obj_t *label = lv_label_create(button);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, C_TEXT, 0);
  lv_obj_center(label);
}

static void build_input_screen() {
  statusbar_create(scr_input);

  lv_obj_t *panel = lv_obj_create(scr_input);
  lv_obj_set_size(panel, LANDSCAPE_W, LANDSCAPE_H - 30);
  lv_obj_set_pos(panel, 0, 30);
  lv_obj_set_style_bg_color(panel, C_BG, 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(panel, 0, 0);
  lv_obj_set_style_radius(panel, 0, 0);
  lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

  /* =====================================================
     ส่วนบนของหน้ากรอกจำนวนเงิน

     Layout:
     - amount_box อยู่ฝั่งซ้าย
     - Scan & Pay อยู่ฝั่งขวา
     - hint อยู่ใต้กล่องจำนวนเงิน
     ===================================================== */
  lv_obj_t *amount_box = lv_obj_create(panel);
  /*
     ปรับขนาดและตำแหน่งกล่องจำนวนเงิน

     lv_obj_set_size(amount_box, 215, 44);
     - 215 = ความกว้าง
     - 44  = ความสูง

     lv_obj_set_pos(amount_box, 38, 8);
     - 38 = ระยะจากซ้าย
     - 8  = ระยะจากด้านบนของ panel

     วิธีแก้:
     - กล่องกว้างขึ้น  -> เพิ่ม 215
     - กล่องแคบลง     -> ลด 215
     - ขยับขวา        -> เพิ่ม 38
     - ขยับซ้าย       -> ลด 38
     - ขยับลง         -> เพิ่ม 8
     - ขยับขึ้น        -> ลด 8
  */
  lv_obj_set_size(amount_box, 215, 44);
  lv_obj_set_pos(amount_box, 38, 8);
  theme_apply_panel(amount_box);
  lv_obj_set_style_bg_color(amount_box, C_PANEL_2, 0);
  lv_obj_set_style_border_color(amount_box, C_BLUE, 0);
  lv_obj_set_style_border_width(amount_box, 1, 0);
  lv_obj_set_style_radius(amount_box, 10, 0);
  lv_obj_clear_flag(amount_box, LV_OBJ_FLAG_SCROLLABLE);

  amount_label = lv_label_create(amount_box);
  lv_obj_set_style_text_color(amount_label, C_BLUE, 0);
  /*
     ขนาดตัวเลขจำนวนเงิน

     &lv_font_montserrat_24 = ตัวอักษรขนาด 24

     หากใหญ่เกินไป:
     เปลี่ยนเป็น &lv_font_montserrat_20

     หากเล็กเกินไป:
     เปลี่ยนเป็น &lv_font_montserrat_28
     แต่ต้องเปิด Font นั้นใน lv_conf.h ก่อน
  */
  lv_obj_set_style_text_font(amount_label, &lv_font_montserrat_24, 0);
  lv_obj_center(amount_label);
  refresh_amount_label();

  lv_obj_t *title = lv_label_create(panel);
  lv_label_set_text(title, "Scan & Pay");
  theme_apply_label(title, true);
  /*
     ชื่อหน้า Scan & Pay

     ขนาด Font:
     &lv_font_montserrat_20

     ตำแหน่ง:
     x = 306
     y = 20

     วิธีแก้:
     - ขยับซ้าย  -> ลด 306
     - ขยับขวา   -> เพิ่ม 306
     - ขยับขึ้น   -> ลด 20
     - ขยับลง    -> เพิ่ม 20
  */
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(title, 306, 20);

  lv_obj_t *hint = lv_label_create(panel);
  lv_label_set_text(hint, "Enter amount (last 2 digits = satang)");
  theme_apply_small_label(hint, true);
  lv_obj_set_pos(hint, 38, 55);

  /* =====================================================
     แป้นตัวเลข 3 x 4 — ขยายปุ่มและจัดเป็นกริดสมมาตร
     ===================================================== */
  static const char *keys[] = {
    "1", "2", "3",
    "4", "5", "6",
    "7", "8", "9",
    "",  "0", "<"
  };

  /*
     ============================
     ปรับแป้นตัวเลขทั้งชุดตรงนี้
     ============================

     btn_w = ความกว้างปุ่ม
     btn_h = ความสูงปุ่ม

     gap_x = ช่องว่างแนวนอนระหว่างปุ่ม
     gap_y = ช่องว่างแนวตั้งระหว่างปุ่ม

     start_x = จุดเริ่มต้นจากซ้าย
     start_y = จุดเริ่มต้นจากด้านบน

     วิธีแก้:
     - ปุ่มใหญ่ขึ้น        -> เพิ่ม btn_w / btn_h
     - ปุ่มเล็กลง         -> ลด btn_w / btn_h
     - ปุ่มห่างกันมากขึ้น -> เพิ่ม gap_x / gap_y
     - ขยับทั้งชุดขวา     -> เพิ่ม start_x
     - ขยับทั้งชุดซ้าย    -> ลด start_x
     - ขยับทั้งชุดลง      -> เพิ่ม start_y
     - ขยับทั้งชุดขึ้น     -> ลด start_y
  */
  const int btn_w = 66;
  const int btn_h = 40;
  const int gap_x = 6;
  const int gap_y = 4;
  const int start_x = 38;
  const int start_y = 76;

  for (int i = 0; i < 12; i++) {
    const int col = i % 3;
    const int row = i / 3;

    lv_obj_t *button = lv_btn_create(panel);
    lv_obj_set_size(button, btn_w, btn_h);
    lv_obj_set_pos(
      button,
      start_x + col * (btn_w + gap_x),
      start_y + row * (btn_h + gap_y)
    );
    lv_obj_set_style_bg_color(button, C_PANEL, 0);
    lv_obj_set_style_bg_color(button, C_PANEL_2, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(button, C_BORDER, 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 9, 0);

    add_button_label(button, keys[i], &lv_font_montserrat_20);

    if (strcmp(keys[i], "<") == 0) {
      lv_obj_add_event_cb(button, backspace_cb, LV_EVENT_CLICKED, NULL);
    } else if (keys[i][0] == '\0') {
      lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_style_bg_opa(button, LV_OPA_30, 0);
    } else {
      lv_obj_add_event_cb(button, numpad_cb, LV_EVENT_CLICKED, (void *)keys[i]);
    }
  }

  /* =====================================================
     ปุ่มด้านขวา — ขนาดเท่ากันและเรียงตรงกัน
     ===================================================== */
  /*
     ============================
     ปรับปุ่ม CREATE QR และ BACK
     ============================

     action_x = ตำแหน่งแนวนอนของปุ่มทั้งสอง
     action_w = ความกว้างปุ่ม
     action_h = ความสูงปุ่ม

     วิธีแก้:
     - ขยับปุ่มทั้งชุดซ้าย  -> ลด action_x
     - ขยับปุ่มทั้งชุดขวา   -> เพิ่ม action_x
     - ปุ่มกว้างขึ้น         -> เพิ่ม action_w
     - ปุ่มสูงขึ้น           -> เพิ่ม action_h
  */
  const int action_x = 290;
  const int action_w = 150;
  const int action_h = 68;

  lv_obj_t *create_button = lv_btn_create(panel);
  lv_obj_set_size(create_button, action_w, action_h);
  /*
     ตำแหน่งปุ่ม CREATE QR

     x = action_x
     y = 82

     ขยับขึ้น -> ลด 82
     ขยับลง  -> เพิ่ม 82
  */
  lv_obj_set_pos(create_button, action_x, 82);
  lv_obj_set_style_bg_color(create_button, C_BLUE, 0);
  lv_obj_set_style_bg_color(create_button, C_BLUE_DARK, LV_STATE_PRESSED);
  lv_obj_set_style_border_width(create_button, 0, 0);
  lv_obj_set_style_radius(create_button, 11, 0);
  lv_obj_add_event_cb(create_button, create_qr_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *create_label = lv_label_create(create_button);
  lv_label_set_text(create_label, "CREATE QR");
  lv_obj_set_style_text_font(create_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(create_label, lv_color_white(), 0);
  lv_obj_center(create_label);

  lv_obj_t *back_button = lv_btn_create(panel);
  lv_obj_set_size(back_button, action_w, action_h);
  /*
     ตำแหน่งปุ่ม BACK

     x = action_x
     y = 166

     ขยับขึ้น -> ลด 166
     ขยับลง  -> เพิ่ม 166

     ระยะห่างระหว่าง CREATE QR และ BACK
     มาจากค่า 166 - 82
  */
  lv_obj_set_pos(back_button, action_x, 166);
  lv_obj_set_style_bg_color(back_button, C_PANEL, 0);
  lv_obj_set_style_bg_color(back_button, C_PANEL_2, LV_STATE_PRESSED);
  lv_obj_set_style_border_color(back_button, C_BORDER, 0);
  lv_obj_set_style_border_width(back_button, 1, 0);
  lv_obj_set_style_radius(back_button, 11, 0);
  lv_obj_add_event_cb(back_button, leave_scan_page_cb, LV_EVENT_CLICKED, NULL);
  add_button_label(back_button, "BACK", &lv_font_montserrat_18);
}

static void scan_page_create_input() {
  display_set_landscape();

  scr_input = lv_obj_create(NULL);
  theme_apply_screen(scr_input);
  build_input_screen();

  lv_scr_load_anim(
    scr_input,
    LV_SCR_LOAD_ANIM_NONE,
    0,
    0,
    true
  );
}

static void show_qr_portrait() {
  const int satang = atoi(amount_buf);

  char amount_text[24];
  snprintf(
    amount_text,
    sizeof(amount_text),
    "%d.%02d",
    satang / 100,
    satang % 100
  );

  char payload[512];
  if (!build_promptpay_payload(payload, sizeof(payload), amount_text)) return;

  display_set_portrait();

  scr_qr = lv_obj_create(NULL);
  lv_obj_set_size(scr_qr, PORTRAIT_W, PORTRAIT_H);
  lv_obj_set_style_bg_color(scr_qr, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(scr_qr, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(scr_qr, 0, 0);
  lv_obj_clear_flag(scr_qr, LV_OBJ_FLAG_SCROLLABLE);

  // แสดงเฉพาะจำนวนเงิน
  lv_obj_t *amount = lv_label_create(scr_qr);
  char display_amount[32];
  snprintf(display_amount, sizeof(display_amount), "%s THB", amount_text);
  lv_label_set_text(amount, display_amount);
  lv_obj_set_style_text_color(amount, C_BLUE, 0);
  lv_obj_set_style_text_font(amount, &lv_font_montserrat_24, 0);
  lv_obj_align(amount, LV_ALIGN_TOP_MID, 0, 24);

  // แสดงเฉพาะ QR ขนาดใหญ่
  lv_obj_t *qr_box = lv_obj_create(scr_qr);
  /*
     ============================
     กล่อง QR Code
     ============================

     ขนาดกล่อง:
     QR_CANVAS_SIZE + 12
     เพิ่ม 12 เพื่อเผื่อขอบรอบ QR

     ตำแหน่ง:
     LV_ALIGN_CENTER = กึ่งกลางจอ
     x = 0
     y = -18

     ค่า y ติดลบ = ขยับขึ้น
     ค่า y เป็นบวก = ขยับลง

     ตัวอย่าง:
     -18 = ขยับขึ้นจากกลาง 18 px
     -30 = ขยับขึ้นมากกว่าเดิม
      0  = อยู่กึ่งกลางพอดี
  */
  lv_obj_set_size(qr_box, QR_CANVAS_SIZE + 12, QR_CANVAS_SIZE + 12);
  lv_obj_align(qr_box, LV_ALIGN_CENTER, 0, -18);
  lv_obj_set_style_bg_color(qr_box, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(qr_box, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(qr_box, 0, 0);
  lv_obj_set_style_pad_all(qr_box, 6, 0);
  lv_obj_clear_flag(qr_box, LV_OBJ_FLAG_SCROLLABLE);

  if (!draw_qr_canvas(qr_box, payload)) {
    display_set_landscape();
    scan_page_create_input();
    return;
  }

  // ปุ่ม Back ด้านล่าง
  lv_obj_t *back_button = lv_btn_create(scr_qr);
  /*
     ปุ่ม BACK ด้านล่างหน้า QR

     ขนาด:
     130 = ความกว้าง
     44  = ความสูง

     ตำแหน่ง:
     LV_ALIGN_BOTTOM_MID = กึ่งกลางด้านล่าง
     x = 0
     y = -22

     ค่า -22 หมายถึงยกขึ้นจากขอบล่าง 22 px

     วิธีแก้:
     - ขยับขึ้นมากขึ้น -> ลดเป็น -30
     - ขยับลง         -> เพิ่มเป็น -15
  */
  lv_obj_set_size(back_button, 130, 44);
  lv_obj_align(back_button, LV_ALIGN_BOTTOM_MID, 0, -22);
  lv_obj_set_style_bg_color(back_button, C_PANEL, 0);
  lv_obj_set_style_border_color(back_button, C_BORDER, 0);
  lv_obj_set_style_border_width(back_button, 1, 0);
  lv_obj_set_style_radius(back_button, 10, 0);
  lv_obj_add_event_cb(back_button, qr_back_cb, LV_EVENT_CLICKED, NULL);
  add_button_label(back_button, "BACK", &lv_font_montserrat_16);

  lv_scr_load_anim(
    scr_qr,
    LV_SCR_LOAD_ANIM_NONE,
    0,
    0,
    true
  );
}

void scan_page_create() {
  amount_len = 0;
  amount_buf[0] = '\0';
  free_qr_buffer();
  scan_page_create_input();
}
