// /******************************************************************************
//  *      INCLUDES
//  *****************************************************************************/

// #include "app_ble_tracking.h"
// #include "app_data.h"

// #include "eddystone_api.h"
// #include "eddystone_protocol.h"
// #include "kalman_filter.h"

// #include "esp_bt.h"
// #include "esp_bt_defs.h"
// #include "esp_bt_main.h"
// #include "esp_gap_ble_api.h"
// #include "esp_gatt_defs.h"
// #include "esp_gattc_api.h"
// #include "esp_log.h"

// #include <string.h>

// /******************************************************************************
//  *    PRIVATE DEFINES
//  *****************************************************************************/

// #define TAG "APP_BLE_TRACKING"

// /******************************************************************************
//  *    PRIVATE TYPEDEFS
//  *****************************************************************************/

// typedef struct ble_tracking_data
// {
//   Kalman_Filter_t s_Kalman_Filter;
//   QueueHandle_t   s_data_scans;
//   QueueHandle_t  *p_data_mqtt_queue;
// } ble_tracking_data_t;

// /******************************************************************************
//  *  PRIVATE PROTOTYPE FUNCTION
//  *****************************************************************************/

// static void APP_BLE_TRACKING_task(void *arg);
// static void APP_BLE_TRACKING_GAP_Callback(esp_gap_ble_cb_event_t  event,
//                                           esp_ble_gap_cb_param_t *param);

// /******************************************************************************
//  *    PRIVATE DATA
//  *****************************************************************************/

// static ble_tracking_data_t   s_ble_tracking_data;
// static esp_ble_scan_params_t ble_scan_params
//     = { .scan_type          = BLE_SCAN_TYPE_ACTIVE,
//         .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
//         .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
//         .scan_interval      = 0x50,
//         .scan_window        = 0x30,
//         .scan_duplicate     = BLE_SCAN_DUPLICATE_DISABLE };

// /******************************************************************************
//  *   PUBLIC FUNCTION
//  *****************************************************************************/

// void
// APP_BLE_TRACKING_CreateTask (void)
// {
//   xTaskCreate(APP_BLE_TRACKING_task, "ble tracking task", 1024, NULL, 7, NULL);
// }

// void
// APP_BLE_TRACKING_Init (void)
// {
//   s_ble_tracking_data.p_data_mqtt_queue = &s_data_system.s_data_mqtt_queue;

//   s_ble_tracking_data.s_data_scans = xQueueCreate(16, sizeof(int8_t));

//   esp_err_t status;

//   ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
//   esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
//   esp_bt_controller_init(&bt_cfg);
//   esp_bt_controller_enable(ESP_BT_MODE_BLE);

//   esp_bluedroid_init();
//   esp_bluedroid_enable();

//   /*<! register the scan callback function to the gap module */
//   if ((status = esp_ble_gap_register_callback(APP_BLE_TRACKING_GAP_Callback))
//       != ESP_OK)
//   {
//     ESP_LOGE(TAG, "gap register error: %s", esp_err_to_name(status));
//     return;
//   }

//   /*<! set scan parameters */
//   esp_ble_gap_set_scan_params(&ble_scan_params);

//   KALMAN_FILTER_Init(&s_ble_tracking_data.s_Kalman_Filter, 2, 2, 0.001, -20);
// }

// /******************************************************************************
//  *  PRIVATE FUNCTION
//  *****************************************************************************/

// static void
// APP_BLE_TRACKING_task (void *arg)
// {
//   int8_t i8_buffer;
//   while (1)
//   {
//     if (xQueueReceive(
//             s_ble_tracking_data.s_data_scans, &i8_buffer, portMAX_DELAY)
//         == pdPASS)
//     {
//       double smoothedRSSI = KALMAN_FILTER_GetRSSI(
//           &s_ble_tracking_data.s_Kalman_Filter, (double)i8_buffer);
//       i8_buffer = (int8_t)smoothedRSSI;
//       xQueueSend(*s_ble_tracking_data.p_data_mqtt_queue, &i8_buffer, 0);
//     }
//   }
// }

// static void
// APP_BLE_TRACKING_GAP_Callback (esp_gap_ble_cb_event_t  event,
//                                esp_ble_gap_cb_param_t *param)
// {
//   esp_err_t err;

//   switch (event)
//   {
//     case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
//       uint32_t duration = 0;
//       esp_ble_gap_start_scanning(duration);
//       break;
//     }
//     case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: {
//       if ((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS)
//       {
//         ESP_LOGE(TAG, "Scan start failed: %s", esp_err_to_name(err));
//       }
//       else
//       {
//         ESP_LOGI(TAG, "Start scanning...");
//       }
//       break;
//     }
//     case ESP_GAP_BLE_SCAN_RESULT_EVT: {
//       esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
//       switch (scan_result->scan_rst.search_evt)
//       {
//         case ESP_GAP_SEARCH_INQ_RES_EVT: {
//           eddystone_result_t eddystone_res;
//           memset(&eddystone_res, 0, sizeof(eddystone_res));
//           esp_err_t ret = eddystone_decode(scan_result->scan_rst.ble_adv,
//                                            scan_result->scan_rst.adv_data_len,
//                                            &eddystone_res);
//           if (ret)
//           {
//             return;
//           }
//           else
//           {
//             // ESP_LOGI(TAG, "--------Eddystone Found----------");
//             // esp_log_buffer_hex("EDDYSTONE_DEMO: Device address:",
//             //                    scan_result->scan_rst.bda,
//             //                    ESP_BD_ADDR_LEN);

//             ESP_LOGI(TAG, "RSSI RAW: %d dbm", scan_result->scan_rst.rssi);
//             xQueueSend(s_ble_tracking_data.s_data_scans,
//                        &scan_result->scan_rst.rssi,
//                        0);
//           }
//           break;
//         }
//         default:
//           break;
//       }
//       break;
//     }
//     default:
//       break;
//   }
// }