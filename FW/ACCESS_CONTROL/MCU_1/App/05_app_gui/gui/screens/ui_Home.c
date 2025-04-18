// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "../ui.h"

void ui_Home_screen_init(void)
{
    ui_Home = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Home, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_Home, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Home, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_HomeText = lv_label_create(ui_Home);
    lv_obj_set_width(ui_HomeText, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_HomeText, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_HomeText, 0);
    lv_obj_set_y(ui_HomeText, -105);
    lv_obj_set_align(ui_HomeText, LV_ALIGN_CENTER);
    lv_label_set_text(ui_HomeText, "Home");
    lv_obj_set_style_text_color(ui_HomeText, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_HomeText, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_HomeText, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_POPUPHomePanel = lv_obj_create(ui_Home);
    lv_obj_set_width(ui_POPUPHomePanel, 220);
    lv_obj_set_height(ui_POPUPHomePanel, 180);
    lv_obj_set_align(ui_POPUPHomePanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_POPUPHomePanel, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_shadow_color(ui_POPUPHomePanel, lv_color_hex(0x969696), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_POPUPHomePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_POPUPHomePanel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_POPUPHomePanel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_POPUPHomePanel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_POPUPHomePanel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_ButtonHomeToMenu = lv_btn_create(ui_Home);
    lv_obj_set_width(ui_ButtonHomeToMenu, 30);
    lv_obj_set_height(ui_ButtonHomeToMenu, 50);
    lv_obj_set_x(ui_ButtonHomeToMenu, 135);
    lv_obj_set_y(ui_ButtonHomeToMenu, 0);
    lv_obj_set_align(ui_ButtonHomeToMenu, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(ui_ButtonHomeToMenu, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ButtonHomeToMenu, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui_ButtonHomeToMenu, &ui_img_1101766569, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui_ButtonHomeToMenu, lv_color_hex(0x969696), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui_ButtonHomeToMenu, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_ButtonHomeToMenu, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui_ButtonHomeToMenu, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui_ButtonHomeToMenu, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui_ButtonHomeToMenu, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_ButtonHomeToMenu, ui_event_ButtonHomeToMenu, LV_EVENT_ALL, NULL);

}
