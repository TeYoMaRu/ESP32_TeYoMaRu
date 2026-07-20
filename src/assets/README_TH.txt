ไฟล์แก้โลโก้พื้นหลังโปร่งใส

ให้นำไฟล์ต่อไปนี้ไปแทนของเดิม:
src/assets/teyomaru_symbol.c
src/assets/teyomaru_logo.h

ใน boot.cpp ใช้เหมือนเดิม:
lv_obj_t *logo_img = lv_img_create(boot_scr);
lv_img_set_src(logo_img, &teyomaru_symbol);
lv_obj_align(logo_img, LV_ALIGN_TOP_MID, 0, 31);

โลโก้ชุดนี้ตัดพื้นหลังแดงและกรอบทึบออกแล้ว
