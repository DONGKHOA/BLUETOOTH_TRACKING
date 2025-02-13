/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_data.h"

#include "app_ble_ibeacon.h"

#include "kalman_filter.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_defs.h"
#include "esp_log.h"

#include <string.h>

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_BLE_IBEACON"

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  uint32_t beacon_addr[2]; // 4 + 2 Bytes Address
  int32_t  rssi;
} beacon_data_t;

typedef struct ble_ibeacon_data
{
  Kalman_Filter_t s_Kalman_Filter;
  QueueHandle_t   s_data_scans;
  QueueHandle_t  *p_data_mqtt_queue;
  beacon_data_t   beacon_buffer[10];
  uint8_t         count;
} ble_ibeacon_data_t;

esp_ble_ibeacon_head_t ibeacon_common_head = { .flags  = { 0x02, 0x01, 0x06 },
                                               .length = 0x1A,
                                               .type   = 0xFF,
                                               .company_id  = 0x004C,
                                               .beacon_type = 0x1502 };

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_BLE_IBEACON_task(void *arg);
static void APP_BLE_IBEACON_GAP_Callback(esp_gap_ble_cb_event_t  event,
                                         esp_ble_gap_cb_param_t *param);
static bool esp_ble_is_ibeacon_packet(uint8_t *adv_data, uint8_t adv_data_len);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static ble_ibeacon_data_t    s_ble_ibeacon_data;
static esp_ble_scan_params_t ble_scan_params
    = { .scan_type          = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval      = 0x50,
        .scan_window        = 0x30,
        .scan_duplicate     = BLE_SCAN_DUPLICATE_DISABLE };

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_BLE_IBEACON_CreateTask (void)
{
  xTaskCreate(APP_BLE_IBEACON_task, "ble ibeacon task", 1024, NULL, 7, NULL);
}

void
APP_BLE_IBEACON_Init (void)
{
  s_ble_ibeacon_data.p_data_mqtt_queue = &s_data_system.s_data_mqtt_queue;

  s_ble_ibeacon_data.s_data_scans = xQueueCreate(16, sizeof(int8_t));

  esp_err_t status;

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_bt_controller_init(&bt_cfg);
  esp_bt_controller_enable(ESP_BT_MODE_BLE);

  esp_bluedroid_init();
  esp_bluedroid_enable();

  /*<! register the scan callback function to the gap module */
  if ((status = esp_ble_gap_register_callback(APP_BLE_IBEACON_GAP_Callback))
      != ESP_OK)
  {
    ESP_LOGE(TAG, "gap register error: %s", esp_err_to_name(status));
    return;
  }

  /*<! set scan parameters */
  esp_ble_gap_set_scan_params(&ble_scan_params);

  KALMAN_FILTER_Init(&s_ble_ibeacon_data.s_Kalman_Filter, 2, 2, 0.001, -20);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_BLE_IBEACON_task (void *arg)
{
  int8_t i8_buffer;
  while (1)
  {
    if (xQueueReceive(
            s_ble_ibeacon_data.s_data_scans, &i8_buffer, portMAX_DELAY)
        == pdPASS)
    {
      double smoothedRSSI = KALMAN_FILTER_GetRSSI(
          &s_ble_ibeacon_data.s_Kalman_Filter, (double)i8_buffer);
      i8_buffer = (int8_t)smoothedRSSI;
      xQueueSend(*s_ble_ibeacon_data.p_data_mqtt_queue, &i8_buffer, 0);
    }
  }
}

static void
APP_BLE_IBEACON_GAP_Callback (esp_gap_ble_cb_event_t  event,
                              esp_ble_gap_cb_param_t *param)
{
  esp_err_t err;

  switch (event)
  {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
      uint32_t duration = 0;
      esp_ble_gap_start_scanning(duration);
      break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: {
      if ((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS)
      {
        ESP_LOGE(TAG, "Scan start failed: %s", esp_err_to_name(err));
      }
      else
      {
        ESP_LOGI(TAG, "Start scanning...");
      }
      break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
      esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
      switch (scan_result->scan_rst.search_evt)
      {
        case ESP_GAP_SEARCH_INQ_RES_EVT: {
          if (esp_ble_is_ibeacon_packet(scan_result->scan_rst.ble_adv,
                                        scan_result->scan_rst.adv_data_len))
          {
            ESP_LOGI(TAG, "----------iBeacon Found----------");

            bool is_existing = false;

            for (int i = 0; i < s_ble_ibeacon_data.count; i++)
            {
              if ((s_ble_ibeacon_data.beacon_buffer[i].beacon_addr[0]
                       == (scan_result->scan_rst.bda[0] << 24)
                   | (scan_result->scan_rst.bda[1] << 16)
                   | (scan_result->scan_rst.bda[2] << 8)
                   | (scan_result->scan_rst.bda[3]))
                  && (s_ble_ibeacon_data.beacon_buffer[i].beacon_addr[1]
                          == (scan_result->scan_rst.bda[4] << 8)
                      | (scan_result->scan_rst.bda[5])))
              {
                is_existing = true;
                break;
              }
            }

            if (!is_existing && s_ble_ibeacon_data.count < 10)
            {
              s_ble_ibeacon_data.beacon_buffer[s_ble_ibeacon_data.count]
                  .beacon_addr[0]
                  = ((scan_result->scan_rst.bda[0] << 24)
                     | (scan_result->scan_rst.bda[1] << 16)
                     | (scan_result->scan_rst.bda[2] << 8)
                     | scan_result->scan_rst.bda[3]);

              s_ble_ibeacon_data.beacon_buffer[s_ble_ibeacon_data.count]
                  .beacon_addr[1]
                  = ((scan_result->scan_rst.bda[4] << 8)
                     | scan_result->scan_rst.bda[5]);

              s_ble_ibeacon_data.beacon_buffer[s_ble_ibeacon_data.count].rssi
                  = scan_result->scan_rst.rssi;

              s_ble_ibeacon_data.count++;
            }

            ESP_LOGI(TAG,
                     "Beacon MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                     scan_result->scan_rst.bda[0],
                     scan_result->scan_rst.bda[1],
                     scan_result->scan_rst.bda[2],
                     scan_result->scan_rst.bda[3],
                     scan_result->scan_rst.bda[4],
                     scan_result->scan_rst.bda[5]);

            ESP_LOGI(TAG, "RSSI RAW: %d dbm", scan_result->scan_rst.rssi);

            beacon_data_t beacon_info;

            beacon_info.beacon_addr[0] = ((scan_result->scan_rst.bda[0] << 24)
                                          | (scan_result->scan_rst.bda[1] << 16)
                                          | (scan_result->scan_rst.bda[2] << 8)
                                          | scan_result->scan_rst.bda[3]);

            beacon_info.beacon_addr[1] = ((scan_result->scan_rst.bda[4] << 8)
                                          | scan_result->scan_rst.bda[5]);

            beacon_info.rssi = scan_result->scan_rst.rssi;

            xQueueSend(s_ble_ibeacon_data.s_data_scans, &beacon_info, 0);
          }
          break;
        }
        default:
          break;
      }
      break;
    }
    default:
      break;
  }
}

static bool
esp_ble_is_ibeacon_packet (uint8_t *adv_data, uint8_t adv_data_len)
{
  bool result = false;

  if ((adv_data != NULL) && (adv_data_len == 0x1E))
  {
    if (!memcmp(adv_data,
                (uint8_t *)&ibeacon_common_head,
                sizeof(ibeacon_common_head)))
    {
      result = true;
    }
  }
  return result;
}