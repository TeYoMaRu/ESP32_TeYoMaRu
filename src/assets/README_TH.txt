วางไฟล์ทั้งหมดใน src/assets/

หน้า Boot ใช้:
#include "../assets/teyomaru_logo.h"

lv_obj_t *logo_img = lv_img_create(boot_scr);
lv_img_set_src(logo_img, &teyomaru_symbol);
lv_obj_align(logo_img, LV_ALIGN_TOP_MID, 0, 31);

พื้นหลังดำถูกทำให้โปร่งใสแล้ว และไม่มีการเพิ่ม Glow
