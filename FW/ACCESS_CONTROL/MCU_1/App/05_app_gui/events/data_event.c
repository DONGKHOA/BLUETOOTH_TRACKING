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

// #include "esp_log.h"

// /******************************************************************************
//  *  PRIVATE DEFINES
//  *****************************************************************************/

// #define MAX_VISIBLE_ITEMS 8   // Maximum items to show
// #define ITEM_HEIGHT       20  // Height of each user item
// #define Y_START           93  // Initial Y position
// #define X_OFFSET          -68 // X position offset

// /******************************************************************************
//  *    PUBLIC DATA
//  *****************************************************************************/

// uint8_t  user_id[MAX_VISIBLE_ITEMS]    = { 0 };
// uint16_t u16_user_len                  = 0;
// char    *user_names[MAX_VISIBLE_ITEMS] = { NULL };

// /******************************************************************************
//  *  PRIVATE PROTOTYPE FUNCTION
//  *****************************************************************************/

// static void EVENT_DATA_LIST_Timer(lv_timer_t *timer);
// static void EVENT_BUTTON_Handler(lv_event_t *e);
// static void EVENT_PROCESS_EVENT_DATA_Task(void *arg);
// static void EVENT_PROCESS_DATA_CreateTask(void);

// /******************************************************************************
//  *    PRIVATE TYPEDEFS
//  *****************************************************************************/

// typedef struct
// {
//   QueueHandle_t *p_send_data_queue;
//   QueueHandle_t *p_receive_data_event_queue;
// } data_event_data_t;

// /******************************************************************************
//  *    PRIVATE DATA
//  *****************************************************************************/

// static lv_obj_t *user_buttons[MAX_VISIBLE_ITEMS];
// static lv_obj_t *id_labels[MAX_VISIBLE_ITEMS];
// static lv_obj_t *name_labels[MAX_VISIBLE_ITEMS];

// static lv_timer_t *timer_data_list;

// static data_event_data_t s_data_event_data;
// static DATA_SYNC_t       s_data_sync;

// static bool b_is_initialize   = false;
// static bool b_is_create_panel = false;

// static char full_name[64] = "";
// static int  full_name_len = 0;
// static int  packet_count  = 0;
// static int  user_index    = 0;
// static int  id_index      = 0;

// /******************************************************************************
//  *   PUBLIC FUNCTION
//  *****************************************************************************/

// void
// EVENT_Data_List (lv_event_t *e)
// {
//   if (b_is_initialize == false)
//   {
//     s_data_event_data.p_receive_data_event_queue
//         = &s_data_system.s_receive_data_event_queue;
//     s_data_event_data.p_send_data_queue = &s_data_system.s_send_data_queue;
//     b_is_initialize                     = true;
//     EVENT_PROCESS_DATA_CreateTask();
//     timer_data_list = lv_timer_create(EVENT_DATA_LIST_Timer, 500, NULL);
//   }

//   s_data_sync.u8_data_start     = DATA_SYNC_REQUEST_USER_DATA;
//   s_data_sync.u8_data_packet[0] = DATA_SYNC_DUMMY;
//   s_data_sync.u8_data_length    = 1;
//   s_data_sync.u8_data_stop      = DATA_STOP_FRAME;

//   // Notify the state of connection to transmit task via queue
//   xQueueSend(*s_data_event_data.p_send_data_queue, &s_data_sync, 0);

//   user_index = 0;
//   for (int i = 0; i < MAX_VISIBLE_ITEMS; i++)
//   {
//     if (user_names[i] != NULL)
//     {
//       free(user_names[i]);
//       user_names[i] = NULL;
//     }
//   }
// }

// /******************************************************************************
//  *   PRIVATE FUNCTION
//  *****************************************************************************/
// static void
// EVENT_PROCESS_DATA_CreateTask (void)
// {
//   xTaskCreate(EVENT_PROCESS_EVENT_DATA_Task,
//               "process event data task",
//               1024 * 4,
//               NULL,
//               6,
//               NULL);
// }

// static void
// EVENT_PROCESS_EVENT_DATA_Task (void *arg)
// {
//   DATA_SYNC_t s_DATA_SYNC;
//   while (1)
//   {
//     if (xQueueReceive(*s_data_event_data.p_receive_data_event_queue,
//                       &s_DATA_SYNC,
//                       portMAX_DELAY)
//         == pdPASS)
//     {
//       switch (s_DATA_SYNC.u8_data_start)
//       {
//         case DATA_SYNC_NUMBER_OF_USER_DATA:
//           u16_user_len = (s_DATA_SYNC.u8_data_packet[0] << 8)
//                          | s_DATA_SYNC.u8_data_packet[1];

//           break;

//         case DATA_SYNC_DETAIL_OF_USER_DATA:

//           bool valid_char_added = false;

//           if (s_DATA_SYNC.u8_data_packet[0] == '\n'
//               && s_DATA_SYNC.u8_data_packet[1] == '\n')
//           {
//             if (user_index < MAX_VISIBLE_ITEMS)
//             {
//               if (user_names[user_index] != NULL)
//               {
//                 free(user_names[user_index]);
//                 user_names[user_index] = NULL;
//               }

//               user_names[user_index] = malloc(strlen(full_name) + 1);
//               if (user_names[user_index] == NULL)
//               {
//                 ESP_LOGE("ERROR",
//                          "Memory allocation failed for user %d",
//                          user_index);
//               }
//               else
//               {
//                 strcpy(user_names[user_index], full_name);
//                 id_index++;
//                 user_index++;
//               }
//             }

//             memset(full_name, 0, sizeof(full_name));
//             full_name_len = 0;
//             packet_count  = 0;
//             break;
//           }

//           if ((s_DATA_SYNC.u8_data_length - 3) == 1)
//           {
//             user_id[id_index] = s_DATA_SYNC.u8_data_packet[0];
//           }

//           for (int i = 0; i < s_DATA_SYNC.u8_data_length; i++)
//           {
//             uint8_t byte = s_DATA_SYNC.u8_data_packet[i];

//             if (byte == 0x00 || byte == 0xA5)
//             {
//               break;
//             }

//             if (byte >= 32 && byte <= 126)
//             {
//               if (packet_count > 0 && !valid_char_added
//                   && full_name_len < sizeof(full_name) - 1)
//               {
//                 full_name[full_name_len++] = ' ';
//               }

//               if (full_name_len < sizeof(full_name) - 1)
//               {
//                 full_name[full_name_len++] = (char)byte;
//                 valid_char_added           = true;
//               }
//             }
//           }

//           if (valid_char_added)
//           {
//             packet_count++;
//           }

//           break;

//         case DATA_SYNC_RESPONSE_ATTENDANCE:
//           // Handle attendance response
//           break;

//         default:
//           break;
//       }
//     }
//   }
// }

// static void
// EVENT_BUTTON_Handler (lv_event_t *e)
// {
//   lv_event_code_t code = lv_event_get_code(e);

//   if (code == LV_EVENT_CLICKED)
//   {
//     UserIDToEnroll(e);
//   }
// }

// static void
// EVENT_DATA_LIST_Timer (lv_timer_t *timer)
// {

//   ui_UserData = lv_obj_create(NULL);
//   lv_obj_remove_style_all(ui_UserData);
//   lv_obj_set_size(ui_UserData, 320, 240);
//   lv_obj_align(ui_UserData, LV_ALIGN_TOP_LEFT, 0, 0);
//   lv_obj_clear_flag(ui_UserData, LV_OBJ_FLAG_SCROLLABLE);

//   ui_Panel4 = lv_obj_create(ui_UserData);
//   lv_obj_set_width(ui_Panel4, 280);
//   lv_obj_set_height(ui_Panel4, 160);
//   lv_obj_align(ui_Panel4, LV_ALIGN_CENTER, 0, 5);

//   lv_obj_set_flex_flow(ui_Panel4, LV_FLEX_FLOW_COLUMN);
//   lv_obj_set_flex_align(
//       ui_Panel4, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
//   lv_obj_clear_flag(ui_Panel4,
//                     LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM
//                         | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
//   lv_obj_set_style_bg_color(
//       ui_Panel4, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_bg_opa(ui_Panel4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_color(
//       ui_Panel4, lv_color_hex(0x969696), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_opa(ui_Panel4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_width(ui_Panel4, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_spread(ui_Panel4, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_ofs_x(ui_Panel4, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_ofs_y(ui_Panel4, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

//   ui_Panel5 = lv_obj_create(ui_UserData);
//   lv_obj_set_width(ui_Panel5, 280);
//   lv_obj_set_height(ui_Panel5, 23);
//   lv_obj_set_x(ui_Panel5, 0);
//   lv_obj_set_y(ui_Panel5, -95);
//   lv_obj_set_align(ui_Panel5, LV_ALIGN_CENTER);
//   lv_obj_clear_flag(ui_Panel5, LV_OBJ_FLAG_SCROLLABLE); /// Flags
//   lv_obj_set_style_bg_color(
//       ui_Panel5, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_bg_opa(ui_Panel5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_color(
//       ui_Panel5, lv_color_hex(0x969696), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_opa(ui_Panel5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_width(ui_Panel5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_spread(ui_Panel5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_ofs_x(ui_Panel5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_ofs_y(ui_Panel5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

//   ui_IDTextUserData = lv_label_create(ui_Panel5);
//   lv_obj_set_width(ui_IDTextUserData, LV_SIZE_CONTENT);  /// 1
//   lv_obj_set_height(ui_IDTextUserData, LV_SIZE_CONTENT); /// 1
//   lv_obj_set_x(ui_IDTextUserData, -5);
//   lv_obj_set_y(ui_IDTextUserData, 0);
//   lv_obj_set_align(ui_IDTextUserData, LV_ALIGN_LEFT_MID);
//   lv_label_set_text(ui_IDTextUserData, "ID");

//   ui_NameTextUserData = lv_label_create(ui_Panel5);
//   lv_obj_set_width(ui_NameTextUserData, LV_SIZE_CONTENT);  /// 1
//   lv_obj_set_height(ui_NameTextUserData, LV_SIZE_CONTENT); /// 1
//   lv_obj_set_x(ui_NameTextUserData, 45);
//   lv_obj_set_y(ui_NameTextUserData, 0);
//   lv_obj_set_align(ui_NameTextUserData, LV_ALIGN_CENTER);
//   lv_label_set_text(ui_NameTextUserData, "Name");
//   lv_obj_set_style_text_font(ui_NameTextUserData,
//                              &lv_font_montserrat_14,
//                              LV_PART_MAIN | LV_STATE_DEFAULT);

//   ui_ESCButton1 = lv_btn_create(ui_UserData);
//   lv_obj_set_width(ui_ESCButton1, 50);
//   lv_obj_set_height(ui_ESCButton1, 20);
//   lv_obj_set_x(ui_ESCButton1, 23);
//   lv_obj_set_y(ui_ESCButton1, 106);
//   lv_obj_set_align(ui_ESCButton1, LV_ALIGN_LEFT_MID);
//   lv_obj_set_style_bg_color(
//       ui_ESCButton1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_bg_opa(ui_ESCButton1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_border_color(
//       ui_ESCButton1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_border_opa(
//       ui_ESCButton1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_border_width(
//       ui_ESCButton1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_color(
//       ui_ESCButton1, lv_color_hex(0x969696), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_opa(
//       ui_ESCButton1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_width(
//       ui_ESCButton1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_spread(
//       ui_ESCButton1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_ofs_x(
//       ui_ESCButton1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_shadow_ofs_y(
//       ui_ESCButton1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_text_font(
//       ui_ESCButton1, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

//   ui_ESCText2 = lv_label_create(ui_ESCButton1);
//   lv_obj_set_width(ui_ESCText2, LV_SIZE_CONTENT);  /// 1
//   lv_obj_set_height(ui_ESCText2, LV_SIZE_CONTENT); /// 1
//   lv_obj_set_align(ui_ESCText2, LV_ALIGN_CENTER);
//   lv_label_set_text(ui_ESCText2, "ESC");
//   lv_obj_set_style_text_color(
//       ui_ESCText2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_text_opa(ui_ESCText2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//   lv_obj_set_style_text_font(
//       ui_ESCText2, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

//   lv_obj_add_event_cb(ui_ESCButton1, ui_event_ESCButton1, LV_EVENT_ALL, NULL);

//   lv_scr_load(ui_UserData);

//   for (int i = 0; i < MAX_VISIBLE_ITEMS; i++)
//   {
//     if (user_buttons[i])
//     {
//       lv_obj_remove_event_cb(user_buttons[i], EVENT_BUTTON_Handler);
//       lv_obj_del(user_buttons[i]);
//       user_buttons[i] = NULL;
//     }
//   }

//   for (int i = 0; i < u16_user_len; i++)
//   {
//     if (user_names[i] != NULL)
//     {
//       user_buttons[i] = lv_btn_create(ui_Panel4);
//       lv_obj_set_size(user_buttons[i], 308, ITEM_HEIGHT);
//       lv_obj_set_pos(
//           user_buttons[i], X_OFFSET, Y_START + (i * (ITEM_HEIGHT + 5)));
//       lv_obj_set_align(user_buttons[i], LV_ALIGN_CENTER);

//       lv_obj_set_scrollbar_mode(ui_Panel4, LV_SCROLLBAR_MODE_ACTIVE);
//       lv_obj_set_scroll_dir(ui_Panel4, LV_DIR_VER);
//       lv_obj_set_flex_flow(ui_Panel4, LV_FLEX_FLOW_COLUMN);

//       lv_obj_set_style_radius(
//           user_buttons[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//       lv_obj_set_style_bg_color(user_buttons[i],
//                                 lv_color_hex(0xFFFFFF),
//                                 LV_PART_MAIN | LV_STATE_DEFAULT);
//       lv_obj_set_style_bg_opa(
//           user_buttons[i], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//       lv_obj_set_style_blend_mode(user_buttons[i],
//                                   LV_BLEND_MODE_NORMAL,
//                                   LV_PART_MAIN | LV_STATE_DEFAULT);

//       id_labels[i] = lv_label_create(user_buttons[i]);
//       lv_obj_set_size(id_labels[i], LV_SIZE_CONTENT, LV_SIZE_CONTENT);
//       lv_obj_set_pos(id_labels[i], -10, 0);
//       lv_obj_set_align(id_labels[i], LV_ALIGN_LEFT_MID);
//       lv_label_set_text_fmt(id_labels[i], "%d", i + 1);
//       lv_obj_set_style_text_color(id_labels[i],
//                                   lv_color_hex(0x000000),
//                                   LV_PART_MAIN | LV_STATE_DEFAULT);
//       lv_obj_set_style_text_opa(
//           id_labels[i], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//       lv_obj_set_style_text_font(id_labels[i],
//                                  &lv_font_montserrat_14,
//                                  LV_PART_MAIN | LV_STATE_DEFAULT);

//       name_labels[i] = lv_label_create(user_buttons[i]);
//       lv_obj_set_size(name_labels[i], LV_SIZE_CONTENT, LV_SIZE_CONTENT);
//       lv_obj_set_pos(name_labels[i], 90, 0);
//       lv_obj_set_align(name_labels[i], LV_ALIGN_LEFT_MID);
//       lv_label_set_text(name_labels[i], user_names[i]);
//       lv_obj_set_style_text_color(name_labels[i],
//                                   lv_color_hex(0x000000),
//                                   LV_PART_MAIN | LV_STATE_DEFAULT);
//       lv_obj_set_style_text_opa(
//           name_labels[i], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//       lv_obj_set_style_text_font(name_labels[i],
//                                  &lv_font_montserrat_14,
//                                  LV_PART_MAIN | LV_STATE_DEFAULT);

//       // lv_obj_add_event_cb(user_buttons[i],
//       //                     EVENT_BUTTON_Handler,
//       //                     LV_EVENT_ALL,
//       //                     (void *)(uintptr_t)i);
//     }
//   }
//   b_is_create_panel = true;
// }