/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "ui.h"
#include "ui_events.h"

#include "app_data.h"

#include "environment.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/******************************************************************************
 *  PUBLIC VARIABLES
 *****************************************************************************/

extern char full_name[64];
extern char enroll_number_id[8];

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

// typedef struct
// {
//   QueueHandle_t *p_send_data_queue;
//   QueueHandle_t *p_receive_data_event_queue;
// } enroll_event_data_t;

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

lv_obj_t *ui_IDTextEnroll2;
lv_obj_t *ui_NumberID;
lv_obj_t *ui_FingerCheck;
lv_obj_t *ui_FaceIDCheck;

char FingerCheck[4];
char FaceIDCheck[4];

static bool b_is_initialize = false;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/
void
EVENT_Enroll_To_UserInfo (lv_event_t *e)
{
  if (b_is_initialize == false)
  {
    b_is_initialize = true;

    ui_IDTextEnroll2 = lv_label_create(ui_UserInfo);
    lv_obj_set_width(ui_IDTextEnroll2, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_IDTextEnroll2, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_IDTextEnroll2, -25);
    lv_obj_set_y(ui_IDTextEnroll2, -75);
    lv_obj_set_align(ui_IDTextEnroll2, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_color(ui_IDTextEnroll2,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_IDTextEnroll2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NumberID = lv_label_create(ui_UserInfo);
    lv_obj_set_width(ui_NumberID, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_NumberID, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_NumberID, -109);
    lv_obj_set_y(ui_NumberID, -74);
    lv_obj_set_align(ui_NumberID, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(
        ui_NumberID, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_NumberID, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_FingerCheck = lv_label_create(ui_UserInfo);
    lv_obj_set_width(ui_FingerCheck, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_FingerCheck, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_FingerCheck, -35);
    lv_obj_set_y(ui_FingerCheck, -41);
    lv_obj_set_align(ui_FingerCheck, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_color(ui_FingerCheck,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_FingerCheck, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_FaceIDCheck = lv_label_create(ui_UserInfo);
    lv_obj_set_width(ui_FaceIDCheck, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_FaceIDCheck, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_FaceIDCheck, -35);
    lv_obj_set_y(ui_FaceIDCheck, -4);
    lv_obj_set_align(ui_FaceIDCheck, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_color(ui_FaceIDCheck,
                                lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(
        ui_FaceIDCheck, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  lv_label_set_text(ui_IDTextEnroll2, full_name);
  lv_label_set_text(ui_NumberID, enroll_number_id);

  lv_label_set_text(ui_FingerCheck, FingerCheck);
  lv_label_set_text(ui_FaceIDCheck, FaceIDCheck);
}