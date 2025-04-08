// /******************************************************************************
//  *      INCLUDES
//  *****************************************************************************/
// #include "ui.h"
// #include "ui_events.h"

// #include "app_data.h"

// #include "environment.h"

// #include <stdbool.h>
// #include <stdio.h>
// #include <string.h>

// /******************************************************************************
//  *    PUBLIC DATA
//  *****************************************************************************/

// uint16_t u16_user_id = 0;

// /******************************************************************************
//  *  PRIVATE PROTOTYPE FUNCTION
//  *****************************************************************************/

// static void EVENT_USER_INFO_Timer(lv_timer_t *timer);

// /******************************************************************************
//  *    PRIVATE DATA
//  *****************************************************************************/

// static lv_timer_t *timer_user_info;

// /******************************************************************************
//  *   PUBLIC FUNCTION
//  *****************************************************************************/
// void
// UserIDToEnroll (lv_event_t *e)
// {
//   // This line is correct to get the index
//   u16_user_id = (uint16_t)(uintptr_t)lv_event_get_user_data(e);

//   // printf("Enroll for user %d\n", index);
//   if (timer_user_info != NULL)
//   {
//     lv_timer_del(timer_user_info);
//     timer_user_info = NULL;
//   }

//   timer_user_info = lv_timer_create(EVENT_USER_INFO_Timer, 100, NULL);
// }
// /******************************************************************************
//  *   PRIVATE FUNCTION
//  *****************************************************************************/
// static void
// EVENT_USER_INFO_Timer (lv_timer_t *timer)
// {
//   lv_timer_del(timer);
//   timer_user_info = NULL;

//   ui_Enroll = lv_obj_create(NULL);
//   lv_obj_remove_style_all(ui_Enroll);
//   lv_obj_set_size(ui_Enroll, 320, 240);
//   lv_obj_align(ui_Enroll, LV_ALIGN_TOP_LEFT, 0, 0);
//   lv_obj_clear_flag(ui_Enroll, LV_OBJ_FLAG_SCROLLABLE);

//   ui_POPUPMenuPanel3 = lv_obj_create(ui_Enroll);
//   lv_obj_set_width(ui_POPUPMenuPanel3, 280);
//   lv_obj_set_height(ui_POPUPMenuPanel3, 180);
//   lv_obj_set_align(ui_POPUPMenuPanel3, LV_ALIGN_CENTER);
//   lv_obj_clear_flag(ui_POPUPMenuPanel3, LV_OBJ_FLAG_SCROLLABLE); /// Flags
//   // lv_obj_set_style_shadow_color(ui_POPUPMenuPanel3,
//   //                               lv_color_hex(0x969696),
//   //                               LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_opa(
//   //     ui_POPUPMenuPanel3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_width(
//   //     ui_POPUPMenuPanel3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_spread(
//   //     ui_POPUPMenuPanel3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_ofs_x(
//   //     ui_POPUPMenuPanel3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_ofs_y(
//   //     ui_POPUPMenuPanel3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

//   ui_IDTextEnroll = lv_label_create(ui_Enroll);
//   lv_obj_set_width(ui_IDTextEnroll, LV_SIZE_CONTENT);  /// 1
//   lv_obj_set_height(ui_IDTextEnroll, LV_SIZE_CONTENT); /// 1
//   lv_obj_set_x(ui_IDTextEnroll, 28);
//   lv_obj_set_y(ui_IDTextEnroll, -74);
//   lv_obj_set_align(ui_IDTextEnroll, LV_ALIGN_LEFT_MID);
//   lv_label_set_text(ui_IDTextEnroll, "ID:");

//   ui_Panel1 = lv_obj_create(ui_Enroll);
//   lv_obj_set_width(ui_Panel1, 261);
//   lv_obj_set_height(ui_Panel1, 27);
//   lv_obj_set_x(ui_Panel1, 0);
//   lv_obj_set_y(ui_Panel1, -42);
//   lv_obj_set_align(ui_Panel1, LV_ALIGN_CENTER);
//   lv_obj_clear_flag(ui_Panel1, LV_OBJ_FLAG_SCROLLABLE); /// Flags
//   lv_obj_set_style_radius(ui_Panel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_bg_color(
//       ui_Panel1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_bg_opa(ui_Panel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

//   ui_Panel3 = lv_obj_create(ui_Enroll);
//   lv_obj_set_width(ui_Panel3, 261);
//   lv_obj_set_height(ui_Panel3, 27);
//   lv_obj_set_x(ui_Panel3, 0);
//   lv_obj_set_y(ui_Panel3, -5);
//   lv_obj_set_align(ui_Panel3, LV_ALIGN_CENTER);
//   lv_obj_clear_flag(ui_Panel3, LV_OBJ_FLAG_SCROLLABLE); /// Flags
//   lv_obj_set_style_radius(ui_Panel3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_bg_color(
//       ui_Panel3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_bg_opa(ui_Panel3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

//   ui_Finger = lv_label_create(ui_Enroll);
//   lv_obj_set_width(ui_Finger, LV_SIZE_CONTENT);  /// 1
//   lv_obj_set_height(ui_Finger, LV_SIZE_CONTENT); /// 1
//   lv_obj_set_x(ui_Finger, 34);
//   lv_obj_set_y(ui_Finger, -41);
//   lv_obj_set_align(ui_Finger, LV_ALIGN_LEFT_MID);
//   lv_label_set_text(ui_Finger, "Finger");

//   ui_FaceID = lv_label_create(ui_Enroll);
//   lv_obj_set_width(ui_FaceID, LV_SIZE_CONTENT);  /// 1
//   lv_obj_set_height(ui_FaceID, LV_SIZE_CONTENT); /// 1
//   lv_obj_set_x(ui_FaceID, 34);
//   lv_obj_set_y(ui_FaceID, -4);
//   lv_obj_set_align(ui_FaceID, LV_ALIGN_LEFT_MID);
//   lv_label_set_text(ui_FaceID, "FaceID");

//   // ui_FingerCheck = lv_label_create(ui_Enroll);
//   // lv_obj_set_width(ui_FingerCheck, LV_SIZE_CONTENT);  /// 1
//   // lv_obj_set_height(ui_FingerCheck, LV_SIZE_CONTENT); /// 1
//   // lv_obj_set_x(ui_FingerCheck, -35);
//   // lv_obj_set_y(ui_FingerCheck, -41);
//   // lv_obj_set_align(ui_FingerCheck, LV_ALIGN_RIGHT_MID);
//   // lv_label_set_text(ui_FingerCheck, "0/1");

//   // ui_FaceIDCheck = lv_label_create(ui_Enroll);
//   // lv_obj_set_width(ui_FaceIDCheck, LV_SIZE_CONTENT);  /// 1
//   // lv_obj_set_height(ui_FaceIDCheck, LV_SIZE_CONTENT); /// 1
//   // lv_obj_set_x(ui_FaceIDCheck, -35);
//   // lv_obj_set_y(ui_FaceIDCheck, -4);
//   // lv_obj_set_align(ui_FaceIDCheck, LV_ALIGN_RIGHT_MID);
//   // lv_label_set_text(ui_FaceIDCheck, "0/1");

//   ui_UserInfo = lv_label_create(ui_Enroll);
//   lv_obj_set_width(ui_UserInfo, LV_SIZE_CONTENT);  /// 1
//   lv_obj_set_height(ui_UserInfo, LV_SIZE_CONTENT); /// 1
//   lv_obj_set_x(ui_UserInfo, 0);
//   lv_obj_set_y(ui_UserInfo, -105);
//   lv_obj_set_align(ui_UserInfo, LV_ALIGN_CENTER);
//   lv_label_set_text(ui_UserInfo, "User Info");

//   ui_EnrollTime = lv_label_create(ui_Enroll);
//   lv_obj_set_width(ui_EnrollTime, LV_SIZE_CONTENT);  /// 1
//   lv_obj_set_height(ui_EnrollTime, LV_SIZE_CONTENT); /// 1
//   lv_obj_set_x(ui_EnrollTime, 23);
//   lv_obj_set_y(ui_EnrollTime, -105);
//   lv_obj_set_align(ui_EnrollTime, LV_ALIGN_LEFT_MID);
//   lv_label_set_text(ui_EnrollTime, "Time");
//   lv_obj_set_style_text_font(
//       ui_EnrollTime, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

//   // ui_ESCButton = lv_btn_create(ui_Enroll);
//   // lv_obj_set_width(ui_ESCButton, 50);
//   // lv_obj_set_height(ui_ESCButton, 20);
//   // lv_obj_set_x(ui_ESCButton, 23);
//   // lv_obj_set_y(ui_ESCButton, 106);
//   // lv_obj_set_align(ui_ESCButton, LV_ALIGN_LEFT_MID);
//   // lv_obj_set_style_bg_color(
//   //     ui_ESCButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_bg_opa(ui_ESCButton, 255, LV_PART_MAIN |
//   // LV_STATE_DEFAULT); lv_obj_set_style_border_color(
//   //     ui_ESCButton, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_border_opa(
//   //     ui_ESCButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_border_width(
//   //     ui_ESCButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_color(
//   //     ui_ESCButton, lv_color_hex(0x969696), LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_opa(
//   //     ui_ESCButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_width(
//   //     ui_ESCButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_spread(
//   //     ui_ESCButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_ofs_x(
//   //     ui_ESCButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_ofs_y(
//   //     ui_ESCButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_text_font(
//   //     ui_ESCButton, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

//   // ui_ESCText = lv_label_create(ui_ESCButton);
//   // lv_obj_set_width(ui_ESCText, LV_SIZE_CONTENT);  /// 1
//   // lv_obj_set_height(ui_ESCText, LV_SIZE_CONTENT); /// 1
//   // lv_obj_set_align(ui_ESCText, LV_ALIGN_CENTER);
//   // lv_label_set_text(ui_ESCText, "ESC");
//   // lv_obj_set_style_text_color(
//   //     ui_ESCText, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_text_opa(ui_ESCText, 255, LV_PART_MAIN |
//   // LV_STATE_DEFAULT); lv_obj_set_style_text_font(
//   //     ui_ESCText, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

//   ui_HomeButton = lv_btn_create(ui_Enroll);
//   lv_obj_set_width(ui_HomeButton, 50);
//   lv_obj_set_height(ui_HomeButton, 20);
//   lv_obj_set_x(ui_HomeButton, -23);
//   lv_obj_set_y(ui_HomeButton, 106);
//   lv_obj_set_align(ui_HomeButton, LV_ALIGN_RIGHT_MID);
//   lv_obj_set_style_bg_color(
//       ui_HomeButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_bg_opa(ui_HomeButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_border_color(
//       ui_HomeButton, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_border_opa(
//       ui_HomeButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_border_width(
//       ui_HomeButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_color(
//   //     ui_HomeButton, lv_color_hex(0x969696), LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_opa(
//   //     ui_HomeButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_width(
//   //     ui_HomeButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_spread(
//   //     ui_HomeButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_ofs_x(
//   //     ui_HomeButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_shadow_ofs_y(
//   //     ui_HomeButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   // lv_obj_set_style_text_font(
//   //     ui_HomeButton, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

//   ui_HomeText1 = lv_label_create(ui_HomeButton);
//   lv_obj_set_width(ui_HomeText1, LV_SIZE_CONTENT);  /// 1
//   lv_obj_set_height(ui_HomeText1, LV_SIZE_CONTENT); /// 1
//   lv_obj_set_align(ui_HomeText1, LV_ALIGN_CENTER);
//   lv_label_set_text(ui_HomeText1, "Home");
//   lv_obj_set_style_text_color(
//       ui_HomeText1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_text_opa(ui_HomeText1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_text_font(
//       ui_HomeText1, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

//   // lv_obj_add_event_cb(ui_ESCButton, ui_event_ESCButton, LV_EVENT_ALL, NULL);
//   lv_obj_add_event_cb(ui_HomeButton, ui_event_HomeButton, LV_EVENT_ALL, NULL);

//   lv_scr_load(ui_Enroll);
// }
