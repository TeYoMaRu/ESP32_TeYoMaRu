/* ======================================================
   scan_page.cpp  –  TeYoMaRu OS
   หน้า Scan โอนเงินผ่าน QR Code (PromptPay / Any2Any)

   ขั้นตอนการทำงาน:
     [1] กรอกจำนวนเงิน (Numpad)
     [2] กด "สร้าง QR" → แสดง QR Code พร้อมจำนวนเงิน
     [3] กด "Back" กลับแก้ไขจำนวน
     [4] กด "ยืนยัน / Done" กลับ Desktop

   ห้องสมุดที่ใช้:
     - LVGL (UI)
     - qrcodegen.h  (สร้าง QR byte array)
       → ใส่ไฟล์ qrcodegen.h + qrcodegen.c ไว้ใน lib/qrcodegen/
         แล้ว add ใน platformio.ini: lib_deps = nayuki/QR-Code-generator

   โครงสร้าง QR payload (PromptPay EMV tag):
     000201
     010212          ← Static QR
     2937            ← Tag 29 (PromptPay merchant)
       0016A000000677010111   ← AID
       011300660000000XXXXXX  ← เบอร์โทร / พร้อมเพย์ 13 หลัก
     5303764          ← สกุลเงิน THB
     54<len><amount>  ← จำนวนเงิน  (tag 54)
     5802TH
     6304<CRC>        ← CRC-16/CCITT

   *** ตัวอย่างนี้ใช้ payload สาธิต (ไม่ใช่บัญชีจริง) ***
   แก้ PROMPTPAY_ID ให้เป็นหมายเลขพร้อมเพย์ของคุณ
====================================================== */

#include "scan_page.h"
#include "desktop.h"
#include "../core/theme.h"
#include "../widgets/statusbar.h"
#include <lvgl.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <esp_heap_caps.h>

// ─── ตั้งค่า PromptPay ───────────────────────────────
//  เปลี่ยนเป็นหมายเลขโทรศัพท์ 10 หลัก (ไม่มี +66)
//  หรือเลขบัตรประชาชน 13 หลัก
#define PROMPTPAY_ID  "0812345678"      // ← แก้ตรงนี้

// ─── ขนาดจอ ─────────────────────────────────────────
#define SCR_W  480
#define SCR_H  320

// ─── สถานะหน้าจอ ─────────────────────────────────────
typedef enum { STATE_INPUT, STATE_QR } ScanState;
static ScanState page_state = STATE_INPUT;

// ─── บัฟเฟอร์จำนวนเงิน ────────────────────────────────
#define AMOUNT_MAX 10              // ไม่เกิน 10 ตัวเลข
static char amount_buf[AMOUNT_MAX + 1] = {0};
static int  amount_len = 0;

// ─── ตัวแปรชี้ไปที่ widget ─────────────────────────────
static lv_obj_t *scr_main   = NULL;   // screen หลัก
static lv_obj_t *panel_input = NULL;  // panel กรอกเงิน
static lv_obj_t *panel_qr    = NULL;  // panel QR

// ─── Forward declarations ─────────────────────────────
static void build_input_panel();
static void build_qr_panel();
static void show_input();
static void show_qr();
static void back_cb(lv_event_t *e);
static void done_cb(lv_event_t *e);
static void gen_qr_cb(lv_event_t *e);
static void numpad_cb(lv_event_t *e);
static void backspace_cb(lv_event_t *e);
static void refresh_amount_label();
static void draw_qr_canvas(lv_obj_t *parent, const char *payload);

/* ─── CRC-16/CCITT (สำหรับ EMV QR) ─────────────────── */
static uint16_t crc16_ccitt(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int j = 0; j < 8; j++) {
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
  }
  return crc;
}

/* ─── สร้าง PromptPay EMV payload ──────────────────── */
static void build_promptpay_payload(char *out, size_t out_size, const char *amount_str) {
  // ปรับเบอร์: ถ้า 10 หลักแปลงเป็น +66XXXXXXXXX (13 หลัก)
  char pid[14] = {0};
  if (strlen(PROMPTPAY_ID) == 10) {
    snprintf(pid, sizeof(pid), "0066%s", PROMPTPAY_ID + 1); // เปลี่ยน 0 → 66
  } else {
    strncpy(pid, PROMPTPAY_ID, 13);
  }

  // Tag 29 payload
  char t29[80];
  snprintf(t29, sizeof(t29), "0016A000000677010111011300%s", pid);
  char tag29[90];
  snprintf(tag29, sizeof(tag29), "29%02zu%s", strlen(t29), t29);

  // Tag 54 (amount)
  char tag54[24];
  snprintf(tag54, sizeof(tag54), "54%02zu%s", strlen(amount_str), amount_str);

  // ประกอบ payload ก่อน CRC
  char body[256];
  snprintf(body, sizeof(body),
    "000201"          // payload format
    "010212"          // point of initiation (static)
    "%s"              // tag 29 (PromptPay)
    "5303764"         // THB
    "%s"              // tag 54 (amount)
    "5802TH"          // country
    "6304",           // CRC tag (ค่า CRC ตามมา)
    tag29, tag54
  );

  uint16_t crc = crc16_ccitt((const uint8_t *)body, strlen(body));

  snprintf(out, out_size, "%s%04X", body, crc);
}

/* ══════════════════════════════════════════════════════
   QR Code  –  วาดด้วย LVGL Canvas (ไม่พึ่งไลบรารีภายนอก)
   ใช้วิธีเข้ารหัส payload เป็น Alphanumeric แบบง่าย
   แล้ว render เป็น grid สีดำ/ขาว

   *** หมายเหตุ: ถ้าต้องการ QR ที่สแกนได้จริง 100% ***
   ให้ใช้ qrcodegen library (Nayuki) แล้วแทนที่ฟังก์ชัน
   draw_qr_canvas() ด้วยการเรียก QrCode_encodeBinary()
   แล้ว loop เขียน pixel ลง canvas

   สำหรับ demo นี้ใช้ lv_canvas วาด pattern จำลอง
   เพื่อให้โปรเจกต์ compile ได้โดยไม่ต้องพึ่ง lib เพิ่ม
══════════════════════════════════════════════════════ */

// ลดขนาด canvas ให้ใช้ RAM น้อยลง (128×128×2 = 32 KB แทน 160×160×2 = 50 KB)
// และจัดสรรบน heap ตอน runtime แทน static เพื่อไม่ค้างใน .bss ตลอด
#define QR_CANVAS_SIZE 128
static lv_color_t *qr_buf = NULL;   // จัดสรรตอน draw, คืนตอน back/done

// ─── Hash อย่างง่ายเพื่อสร้าง pattern จำลองให้ดูต่างกัน ───
static uint32_t simple_hash(const char *s) {
  uint32_t h = 5381;
  while (*s) h = ((h << 5) + h) ^ (uint8_t)(*s++);
  return h;
}

static void draw_qr_canvas(lv_obj_t *parent, const char *payload) {
  // จัดสรร buffer บน heap ตอน runtime เพื่อไม่ให้ค้างใน .bss/.data
  if (!qr_buf) {
    qr_buf = (lv_color_t *)heap_caps_malloc(
      QR_CANVAS_SIZE * QR_CANVAS_SIZE * sizeof(lv_color_t),
      MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL
    );
    if (!qr_buf) return;  // RAM ไม่พอ
  }
  lv_obj_t *canvas = lv_canvas_create(parent);
  lv_canvas_set_buffer(canvas, qr_buf, QR_CANVAS_SIZE, QR_CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);
  lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);

  // พื้นขาว
  lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER);

  uint32_t seed = simple_hash(payload);
  int cells = 29;                    // 29×29 grid (Version 3 QR)
  int cell_px = QR_CANVAS_SIZE / cells;
  int margin  = (QR_CANVAS_SIZE - cells * cell_px) / 2;

  lv_draw_rect_dsc_t rdsc;
  lv_draw_rect_dsc_init(&rdsc);

  for (int r = 0; r < cells; r++) {
    for (int c = 0; c < cells; c++) {
      // Finder pattern  (3 มุม)
      bool finder = false;
      if ((r < 7 && c < 7) || (r < 7 && c >= cells-7) || (r >= cells-7 && c < 7))
        finder = true;

      bool dark = finder;
      if (!dark) {
        // pseudo-random จาก seed + payload
        uint32_t v = seed ^ ((uint32_t)r * 31 + c) * 2654435761UL;
        dark = (v & 1);
      }

      rdsc.bg_color = dark ? lv_color_black() : lv_color_white();
      rdsc.bg_opa   = LV_OPA_COVER;
      rdsc.border_width = 0;
      rdsc.radius   = 0;

      lv_area_t a;
      a.x1 = margin + c * cell_px;
      a.y1 = margin + r * cell_px;
      a.x2 = a.x1 + cell_px - 1;
      a.y2 = a.y1 + cell_px - 1;
      lv_canvas_draw_rect(canvas, a.x1, a.y1, cell_px, cell_px, &rdsc);
    }
  }
}

/* ══════════════════════════════════════════════════════
   Input Panel  (ขั้นตอนที่ 1 – กรอกจำนวนเงิน)
══════════════════════════════════════════════════════ */

static lv_obj_t *amount_label = NULL;   // แสดงจำนวนที่กรอก

static void refresh_amount_label() {
  if (!amount_label) return;
  char buf[32];
  if (amount_len == 0) {
    lv_label_set_text(amount_label, "0.00 THB");
  } else {
    // แสดงทศนิยม 2 ตำแหน่ง: เช่น "10050" → "100.50 THB"
    int satang = atoi(amount_buf);
    snprintf(buf, sizeof(buf), "%d.%02d THB", satang / 100, satang % 100);
    lv_label_set_text(amount_label, buf);
  }
}

static void numpad_cb(lv_event_t *e) {
  const char *digit = (const char *)lv_event_get_user_data(e);
  if (amount_len >= AMOUNT_MAX) return;
  amount_buf[amount_len++] = digit[0];
  amount_buf[amount_len]   = '\0';
  refresh_amount_label();
}

static void backspace_cb(lv_event_t *e) {
  (void)e;
  if (amount_len > 0) {
    amount_buf[--amount_len] = '\0';
    refresh_amount_label();
  }
}

static void gen_qr_cb(lv_event_t *e) {
  (void)e;
  if (amount_len == 0) return;   // ยังไม่กรอก → ไม่ทำอะไร
  show_qr();
}

static void build_input_panel() {
  panel_input = lv_obj_create(scr_main);
  lv_obj_set_size(panel_input, SCR_W, SCR_H - 30); // ลบ statusbar
  lv_obj_set_pos(panel_input, 0, 30);
  lv_obj_set_style_bg_color(panel_input, C_BG, 0);
  lv_obj_set_style_bg_opa(panel_input, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(panel_input, 0, 0);
  lv_obj_set_style_radius(panel_input, 0, 0);
  lv_obj_clear_flag(panel_input, LV_OBJ_FLAG_SCROLLABLE);

  // ─── หัวข้อ ───
  lv_obj_t *title = lv_label_create(panel_input);
  lv_label_set_text(title, "Scan & Pay");
  theme_apply_label(title, true);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

  // ─── กล่องแสดงจำนวนเงิน ───
  lv_obj_t *amount_box = lv_obj_create(panel_input);
  lv_obj_set_size(amount_box, 240, 44);
  lv_obj_align(amount_box, LV_ALIGN_TOP_MID, 0, 38);
  theme_apply_panel(amount_box);
  lv_obj_set_style_bg_color(amount_box, C_PANEL_2, 0);
  lv_obj_set_style_border_color(amount_box, C_BLUE, 0);
  lv_obj_clear_flag(amount_box, LV_OBJ_FLAG_SCROLLABLE);

  amount_label = lv_label_create(amount_box);
  lv_label_set_text(amount_label, "0.00 THB");
  lv_obj_set_style_text_color(amount_label, C_BLUE, 0);
  lv_obj_set_style_text_font(amount_label, &lv_font_montserrat_20, 0);
  lv_obj_center(amount_label);

  // ─── คำอธิบายขนาดเล็ก ───
  lv_obj_t *hint = lv_label_create(panel_input);
  lv_label_set_text(hint, "กรอกจำนวนเงิน (สตางค์ 2 ตำแหน่ง)");
  theme_apply_small_label(hint, true);
  lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 88);

  // ─── Numpad  3×4  (1-9, ., 0, ⌫) ───
  static const char *keys[] = {
    "1","2","3",
    "4","5","6",
    "7","8","9",
    ".","0","<"
  };

  int btn_w = 58, btn_h = 42, gap_x = 8, gap_y = 6;
  int start_x = (240 - (3*btn_w + 2*gap_x)) / 2 + 0;   // กึ่งกลาง panel_input
  int start_y = 100;

  for (int i = 0; i < 12; i++) {
    int col = i % 3, row = i / 3;
    int bx = start_x + col * (btn_w + gap_x);
    int by = start_y + row * (btn_h + gap_y);

    lv_obj_t *btn = lv_btn_create(panel_input);
    lv_obj_set_size(btn, btn_w, btn_h);
    lv_obj_set_pos(btn, bx + (SCR_W - 240) / 2 - 20, by);
    lv_obj_set_style_bg_color(btn, C_PANEL, 0);
    lv_obj_set_style_border_color(btn, C_BORDER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_radius(btn, 8, 0);
    lv_obj_set_style_bg_color(btn, C_PANEL_2, LV_STATE_PRESSED);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, keys[i]);
    lv_obj_set_style_text_color(lbl, C_TEXT, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, 0);
    lv_obj_center(lbl);

    if (strcmp(keys[i], "<") == 0) {
      lv_obj_set_style_text_color(lbl, C_RED, 0);
      lv_obj_add_event_cb(btn, backspace_cb, LV_EVENT_CLICKED, NULL);
    } else if (strcmp(keys[i], ".") == 0) {
      // ปุ่ม . ไม่ทำงาน (ทศนิยมคำนวณอัตโนมัติ) แต่แสดงให้สวย
      lv_obj_set_style_bg_color(btn, C_CARD, 0);
      lv_obj_set_style_text_color(lbl, C_MUTED, 0);
    } else {
      lv_obj_add_event_cb(btn, numpad_cb, LV_EVENT_CLICKED, (void *)keys[i]);
    }
  }

  // ─── ปุ่ม สร้าง QR ───
  lv_obj_t *qr_btn = lv_btn_create(panel_input);
  lv_obj_set_size(qr_btn, 140, 38);
  lv_obj_align(qr_btn, LV_ALIGN_BOTTOM_MID, 0, -8);
  lv_obj_set_style_bg_color(qr_btn, C_BLUE, 0);
  lv_obj_set_style_bg_color(qr_btn, C_BLUE_DARK, LV_STATE_PRESSED);
  lv_obj_set_style_radius(qr_btn, 10, 0);
  lv_obj_set_style_border_width(qr_btn, 0, 0);
  lv_obj_add_event_cb(qr_btn, gen_qr_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *qr_lbl = lv_label_create(qr_btn);
  lv_label_set_text(qr_lbl, "สร้าง QR Code");
  lv_obj_set_style_text_color(qr_lbl, C_BG, 0);
  lv_obj_set_style_text_font(qr_lbl, &lv_font_montserrat_16, 0);
  lv_obj_center(qr_lbl);

  // ─── ปุ่ม Back ───
  lv_obj_t *back_btn = lv_btn_create(panel_input);
  lv_obj_set_size(back_btn, 80, 32);
  lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 8, -12);
  lv_obj_set_style_bg_color(back_btn, C_PANEL, 0);
  lv_obj_set_style_border_color(back_btn, C_BORDER, 0);
  lv_obj_set_style_border_width(back_btn, 1, 0);
  lv_obj_set_style_radius(back_btn, 8, 0);
  lv_obj_add_event_cb(back_btn, back_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *back_lbl = lv_label_create(back_btn);
  lv_label_set_text(back_lbl, "Back");
  lv_obj_set_style_text_color(back_lbl, C_TEXT, 0);
  lv_obj_set_style_text_font(back_lbl, &lv_font_montserrat_14, 0);
  lv_obj_center(back_lbl);
}

/* ══════════════════════════════════════════════════════
   QR Panel  (ขั้นตอนที่ 2 – แสดง QR Code)
══════════════════════════════════════════════════════ */

static void build_qr_panel() {
  panel_qr = lv_obj_create(scr_main);
  lv_obj_set_size(panel_qr, SCR_W, SCR_H - 30);
  lv_obj_set_pos(panel_qr, 0, 30);
  lv_obj_set_style_bg_color(panel_qr, C_BG, 0);
  lv_obj_set_style_bg_opa(panel_qr, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(panel_qr, 0, 0);
  lv_obj_set_style_radius(panel_qr, 0, 0);
  lv_obj_clear_flag(panel_qr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);   // ซ่อนก่อน

  // ─── หัวข้อ ───
  lv_obj_t *title = lv_label_create(panel_qr);
  lv_label_set_text(title, "สแกน QR โอนเงิน");
  theme_apply_label(title, true);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

  // ─── กล่องแสดงจำนวน ───
  lv_obj_t *amt_box = lv_obj_create(panel_qr);
  lv_obj_set_size(amt_box, 200, 34);
  lv_obj_align(amt_box, LV_ALIGN_TOP_MID, 0, 34);
  theme_apply_panel(amt_box);
  lv_obj_set_style_bg_color(amt_box, C_PANEL_2, 0);
  lv_obj_set_style_border_color(amt_box, C_GREEN, 0);
  lv_obj_clear_flag(amt_box, LV_OBJ_FLAG_SCROLLABLE);

  // Label แสดงจำนวนเงิน (จะอัปเดตตอน show_qr())
  lv_obj_t *amt_lbl = lv_label_create(amt_box);
  lv_label_set_text(amt_lbl, "—");
  lv_obj_set_style_text_color(amt_lbl, C_GREEN, 0);
  lv_obj_set_style_text_font(amt_lbl, &lv_font_montserrat_16, 0);
  lv_obj_center(amt_lbl);
  // ตั้งชื่อเพื่อค้นหาทีหลัง
  lv_obj_set_user_data(amt_lbl, (void *)"qr_amt_lbl");

  // ─── กล่อง QR ───
  lv_obj_t *qr_box = lv_obj_create(panel_qr);
  lv_obj_set_size(qr_box, QR_CANVAS_SIZE + 16, QR_CANVAS_SIZE + 16);
  lv_obj_align(qr_box, LV_ALIGN_CENTER, -40, 8);
  theme_apply_panel(qr_box);
  lv_obj_set_style_bg_color(qr_box, lv_color_white(), 0);
  lv_obj_set_style_border_color(qr_box, C_BLUE, 0);
  lv_obj_set_style_border_width(qr_box, 2, 0);
  lv_obj_set_style_radius(qr_box, 10, 0);
  lv_obj_set_style_pad_all(qr_box, 8, 0);
  lv_obj_clear_flag(qr_box, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_user_data(qr_box, (void *)"qr_box");

  // ─── PromptPay logo text ───
  lv_obj_t *pp_lbl = lv_label_create(panel_qr);
  lv_label_set_text(pp_lbl, "PromptPay");
  lv_obj_set_style_text_color(pp_lbl, C_ORANGE, 0);
  lv_obj_set_style_text_font(pp_lbl, &lv_font_montserrat_14, 0);
  lv_obj_align(pp_lbl, LV_ALIGN_CENTER, -40, (QR_CANVAS_SIZE / 2) + 18);

  // ─── ข้อความข้างๆ ───
  lv_obj_t *info_box = lv_obj_create(panel_qr);
  lv_obj_set_size(info_box, 150, QR_CANVAS_SIZE + 16);
  lv_obj_align(info_box, LV_ALIGN_CENTER, 90, 8);
  theme_apply_panel(info_box);
  lv_obj_set_style_bg_color(info_box, C_PANEL, 0);
  lv_obj_clear_flag(info_box, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *i1 = lv_label_create(info_box);
  lv_label_set_text(i1, "เปิดแอปธนาคาร\nแล้วสแกน QR\nด้านซ้าย");
  lv_obj_set_style_text_color(i1, C_TEXT, 0);
  lv_obj_set_style_text_font(i1, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_align(i1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(i1, LV_ALIGN_TOP_MID, 0, 10);

  lv_obj_t *i2 = lv_label_create(info_box);
  lv_label_set_text(i2, "รับเงินเข้า\nพร้อมเพย์\nทันที");
  lv_obj_set_style_text_color(i2, C_GREEN, 0);
  lv_obj_set_style_text_font(i2, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_align(i2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(i2, LV_ALIGN_BOTTOM_MID, 0, -10);

  // ─── ปุ่ม กลับแก้ไข ───
  lv_obj_t *edit_btn = lv_btn_create(panel_qr);
  lv_obj_set_size(edit_btn, 110, 34);
  lv_obj_align(edit_btn, LV_ALIGN_BOTTOM_LEFT, 8, -8);
  lv_obj_set_style_bg_color(edit_btn, C_PANEL, 0);
  lv_obj_set_style_border_color(edit_btn, C_BORDER, 0);
  lv_obj_set_style_border_width(edit_btn, 1, 0);
  lv_obj_set_style_radius(edit_btn, 8, 0);
  lv_obj_add_event_cb(edit_btn, [](lv_event_t *ev) {
    (void)ev;
    show_input();
  }, LV_EVENT_CLICKED, NULL);

  lv_obj_t *edit_lbl = lv_label_create(edit_btn);
  lv_label_set_text(edit_lbl, "< แก้จำนวน");
  lv_obj_set_style_text_color(edit_lbl, C_TEXT, 0);
  lv_obj_set_style_text_font(edit_lbl, &lv_font_montserrat_14, 0);
  lv_obj_center(edit_lbl);

  // ─── ปุ่ม Done ───
  lv_obj_t *done_btn = lv_btn_create(panel_qr);
  lv_obj_set_size(done_btn, 110, 34);
  lv_obj_align(done_btn, LV_ALIGN_BOTTOM_RIGHT, -8, -8);
  lv_obj_set_style_bg_color(done_btn, C_GREEN, 0);
  lv_obj_set_style_radius(done_btn, 8, 0);
  lv_obj_set_style_border_width(done_btn, 0, 0);
  lv_obj_add_event_cb(done_btn, done_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *done_lbl = lv_label_create(done_btn);
  lv_label_set_text(done_lbl, "Done ✓");
  lv_obj_set_style_text_color(done_lbl, lv_color_white(), 0);
  lv_obj_set_style_text_font(done_lbl, &lv_font_montserrat_16, 0);
  lv_obj_center(done_lbl);
}

/* ─── สลับระหว่าง Input ↔ QR ─────────────────────── */

static void show_input() {
  page_state = STATE_INPUT;
  lv_obj_clear_flag(panel_input, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(panel_qr,    LV_OBJ_FLAG_HIDDEN);
  refresh_amount_label();
}

static void show_qr() {
  page_state = STATE_QR;

  // สร้าง payload
  int satang = atoi(amount_buf);
  char amount_str[20];
  snprintf(amount_str, sizeof(amount_str), "%d.%02d", satang / 100, satang % 100);

  char payload[512];
  build_promptpay_payload(payload, sizeof(payload), amount_str);

  // อัปเดต label จำนวนเงินใน QR panel
  lv_obj_t *amt_lbl = lv_obj_get_child(lv_obj_get_child(panel_qr, 1), 0);
  if (amt_lbl) {
    char show_buf[32];
    snprintf(show_buf, sizeof(show_buf), "%s THB", amount_str);
    lv_label_set_text(amt_lbl, show_buf);
  }

  // วาด QR ใหม่ใน qr_box (ลบ child เก่าก่อน)
  lv_obj_t *qr_box = lv_obj_get_child(panel_qr, 2);
  if (qr_box) {
    lv_obj_clean(qr_box);
    draw_qr_canvas(qr_box, payload);
  }

  lv_obj_add_flag(panel_input, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
}

/* ─── Callbacks ──────────────────────────────────── */

static void back_cb(lv_event_t *e) {
  (void)e;
  // คืน heap buffer ก่อนออกจากหน้า
  if (qr_buf) { heap_caps_free(qr_buf); qr_buf = NULL; }
  amount_len = 0;
  amount_buf[0] = '\0';
  desktop_create();
}

static void done_cb(lv_event_t *e) {
  (void)e;
  if (qr_buf) { heap_caps_free(qr_buf); qr_buf = NULL; }
  amount_len = 0;
  amount_buf[0] = '\0';
  desktop_create();
}

/* ══════════════════════════════════════════════════════
   Entry point
══════════════════════════════════════════════════════ */
void scan_page_create() {
  // รีเซ็ตสถานะ
  amount_len = 0;
  amount_buf[0] = '\0';
  page_state = STATE_INPUT;

  scr_main = lv_obj_create(NULL);
  theme_apply_screen(scr_main);

  statusbar_create(scr_main);
  build_input_panel();
  build_qr_panel();

  lv_scr_load(scr_main);
}
