/******************************************************************************
 *      INCLUDES
 ******************************************************************************/

#include <string.h>

#include "app_data.h"
#include "app_ble_ibeacon.h"

#include "kalman_filter.h"
#include "linked_list.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_defs.h"
#include "esp_log.h"

#include "freertos/timers.h"

/******************************************************************************
 *    PUBLIC DATA
 *****************************************************************************/

uint32_t u32_number_tag = 0;

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG         "APP_BLE_IBEACON"
#define TIME_DELETE 30000 // X2

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

/**
 * @brief Structure defining the data heading of Bluetooth beacon.
 */
typedef struct
{
  uint8_t  flags[3];
  uint8_t  length;
  uint8_t  type;
  uint16_t company_id;
  uint16_t beacon_type;
} __attribute__((packed)) esp_ble_ibeacon_head_t;

/**
 * @brief Structure defining the data of node in linked list.
 */
typedef struct
{
  Kalman_Filter_t s_Kalman_Filter;
  double          filtered_rssi;
  uint8_t         u8_beacon_addr[6]; // 6 Bytes Address
  int8_t          rssi;
  uint8_t         flag_timeout : 1;
} beacon_data_t;

/**
 * @brief Structure defining the data sync between the callback and task.
 */
typedef struct
{
  uint8_t u8_beacon_addr[6]; // 6 Bytes Address
  int8_t  rssi;
} beacon_data_cb_t;

/**
 * @brief Structure defining the data of app_ble_ibeacon.
 */
typedef struct ble_ibeacon_data
{
  TimerHandle_t      s_timeout_delete_node;
  QueueHandle_t      s_queue_ble_inf;
  QueueHandle_t     *p_rssi_ibeacon_queue;
  SemaphoreHandle_t *p_mutex_num_tag;
} ble_ibeacon_data_t;

esp_ble_ibeacon_head_t ibeacon_common_head = { .flags  = { 0x02, 0x01, 0x06 },
                                               .length = 0x1A,
                                               .type   = 0xFF,
                                               .company_id  = 0x004C,
                                               .beacon_type = 0x1502 };

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void    APP_BLE_IBEACON_TimerCallback(TimerHandle_t s_Timer);
static void    APP_BLE_IBEACON_Task(void *arg);
static uint8_t APP_BLE_IBEACON_CheckAddressDevice(beacon_data_t *p_data);
static void    APP_BLE_IBEACON_UpdateDataDevice(beacon_data_t *p_data);
static void    APP_BLE_IBEACON_GAP_Callback(esp_gap_ble_cb_event_t  event,
                                            esp_ble_gap_cb_param_t *param);
static bool esp_ble_is_ibeacon_packet(uint8_t *adv_data, uint8_t adv_data_len);

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static ble_ibeacon_data_t    s_ble_ibeacon_data;
static Node_t               *p_head_linked_list_ble_inf = NULL;
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
  xTaskCreate(
      APP_BLE_IBEACON_Task, "ble ibeacon task", 1024 * 20, NULL, 12, NULL);
}

void
APP_BLE_IBEACON_Init (void)
{
  s_ble_ibeacon_data.p_rssi_ibeacon_queue = &s_data_system.s_rssi_ibeacon_queue;
  s_ble_ibeacon_data.p_mutex_num_tag      = &s_data_system.s_mutex_num_tag;

  s_ble_ibeacon_data.s_timeout_delete_node
      = xTimerCreate("timeout delete node",
                     TIME_DELETE / portTICK_PERIOD_MS,
                     pdTRUE,
                     (void *)0,
                     APP_BLE_IBEACON_TimerCallback);

  s_ble_ibeacon_data.s_queue_ble_inf
      = xQueueCreate(16, sizeof(beacon_data_cb_t));

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_bt_controller_init(&bt_cfg);
  esp_bt_controller_enable(ESP_BT_MODE_BLE);

  esp_bluedroid_init();
  esp_bluedroid_enable();

  /*<! register the scan callback function to the gap module */
  esp_ble_gap_register_callback(APP_BLE_IBEACON_GAP_Callback);

  /*<! set scan parameters */
  esp_ble_gap_set_scan_params(&ble_scan_params);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_BLE_IBEACON_TimerCallback (TimerHandle_t s_Timer)
{
  uint32_t u32_index = 1;
  Node_t  *p_temp    = p_head_linked_list_ble_inf;

  while (p_temp != NULL)
  {
    if (((beacon_data_t *)(p_temp->p_data))->flag_timeout == 1)
    {
      ((beacon_data_t *)(p_temp->p_data))->flag_timeout = 0;
    }
    else if (((beacon_data_t *)(p_temp->p_data))->flag_timeout == 0)
    {
      printf("\r\nDelete Node:");
      for (uint8_t i = 0; i < 6; i++)
      {
        printf("%02X ", ((beacon_data_t *)(p_temp->p_data))->u8_beacon_addr[i]);
      }
      printf("\r\n");

      p_head_linked_list_ble_inf
          = LINKED_LIST_DeleteNode(p_head_linked_list_ble_inf, u32_index);

      xSemaphoreTake(*s_ble_ibeacon_data.p_mutex_num_tag, portMAX_DELAY);
      u32_number_tag--;
      xSemaphoreGive(*s_ble_ibeacon_data.p_mutex_num_tag);

      u32_index = 1;
      p_temp    = p_head_linked_list_ble_inf;
      continue;
    }

    u32_index++;
    p_temp = p_temp->p_next;
  }
}

static void
APP_BLE_IBEACON_Task (void *arg)
{
  beacon_data_t    s_beacon_data_task;
  beacon_data_cb_t s_beacon_data_cb;
  Node_t          *p_temp = p_head_linked_list_ble_inf;
  while (1)
  {
    if (xQueueReceive(s_ble_ibeacon_data.s_queue_ble_inf,
                      &s_beacon_data_cb,
                      portMAX_DELAY)
        == pdPASS)
    {
      printf("\r\nNumber of Tag: %ld\r\n", u32_number_tag);
      memcpy(s_beacon_data_task.u8_beacon_addr,
             s_beacon_data_cb.u8_beacon_addr,
             6);
      s_beacon_data_task.rssi = s_beacon_data_cb.rssi;

      if ((u32_number_tag == 0)
          || (APP_BLE_IBEACON_CheckAddressDevice(&s_beacon_data_task) == 0))
      {
        printf("Create Node:");
        for (uint8_t i = 0; i < 6; i++)
        {
          printf("%02X ", s_beacon_data_cb.u8_beacon_addr[i]);
        }
        printf("\r\n");

        if (u32_number_tag == 0)
        {
          xTimerStart(s_ble_ibeacon_data.s_timeout_delete_node, 0);
          LINKED_LIST_InsertAtHead(&p_head_linked_list_ble_inf,
                                   &s_beacon_data_task,
                                   sizeof(beacon_data_t));
        }
        else
        {
          LINKED_LIST_InsertAtTail(&p_head_linked_list_ble_inf,
                                   &s_beacon_data_task,
                                   sizeof(beacon_data_t));
        }

        p_temp = p_head_linked_list_ble_inf;

        for (uint32_t i = 0; i < u32_number_tag; i++)
        {
          p_temp = p_temp->p_next;
        }

        KALMAN_FILTER_Init(
            &((beacon_data_t *)(p_temp->p_data))->s_Kalman_Filter,
            2,
            2,
            0.001,
            -50);
        ((beacon_data_t *)(p_temp->p_data))->flag_timeout = 0;

        xSemaphoreTake(*s_ble_ibeacon_data.p_mutex_num_tag, portMAX_DELAY);
        u32_number_tag++;
        xSemaphoreGive(*s_ble_ibeacon_data.p_mutex_num_tag);
      }

      APP_BLE_IBEACON_UpdateDataDevice(&s_beacon_data_task);
    }
  }
}

// 1 -> stored
// 0 -> not stored
static uint8_t
APP_BLE_IBEACON_CheckAddressDevice (beacon_data_t *p_data)
{
  Node_t *p_temp = p_head_linked_list_ble_inf;

  while (p_temp != NULL)
  {
    if (memcmp(((beacon_data_t *)(p_temp->p_data))->u8_beacon_addr,
               p_data->u8_beacon_addr,
               6)
        == 0)
    {
      return 1;
    }
    p_temp = p_temp->p_next;
  }
  return 0;
}

static void
APP_BLE_IBEACON_UpdateDataDevice (beacon_data_t *p_data)
{
  ibeacon_infor_tag_t s_beacon_infor_tag;
  uint8_t             u8_count = 0;
  Node_t             *p_temp   = p_head_linked_list_ble_inf;
  while (p_temp != NULL)
  {
    if (memcmp(((beacon_data_t *)(p_temp->p_data))->u8_beacon_addr,
               p_data->u8_beacon_addr,
               6)
        == 0)
    {
      memcpy(((beacon_data_t *)(p_temp->p_data))->u8_beacon_addr,
             p_data->u8_beacon_addr,
             6);

      ((beacon_data_t *)(p_temp->p_data))->rssi = p_data->rssi;

      ((beacon_data_t *)(p_temp->p_data))->filtered_rssi
          = KALMAN_FILTER_GetRSSI(
              &((beacon_data_t *)(p_temp->p_data))->s_Kalman_Filter,
              (double)((beacon_data_t *)(p_temp->p_data))->rssi);

      ((beacon_data_t *)(p_temp->p_data))->flag_timeout = 1;

      printf("\r\n");
      ESP_LOGI(TAG, "----------iBeacon Found----------");
      for (uint8_t i = 0; i < 6; i++)
      {
        printf("%02X ", p_data->u8_beacon_addr[i]);
        s_beacon_infor_tag.u8_beacon_addr[i] = p_data->u8_beacon_addr[i];
      }
      printf("\r\n");

      s_beacon_infor_tag.i8_filtered_rssi
          = (int8_t)((beacon_data_t *)(p_temp->p_data))->filtered_rssi;
      xQueueSend(*s_ble_ibeacon_data.p_rssi_ibeacon_queue,
                 (void *)&s_beacon_infor_tag,
                 (TickType_t)0);

      return;
    }

    u8_count++;
    p_temp = p_temp->p_next;
  }
}

static void
APP_BLE_IBEACON_GAP_Callback (esp_gap_ble_cb_event_t  event,
                              esp_ble_gap_cb_param_t *param)
{
  esp_err_t        err;
  beacon_data_cb_t s_beacon_data_cb;
  switch (event)
  {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
      esp_ble_gap_start_scanning((uint32_t)0);
      break;

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
      if ((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS)
      {
        ESP_LOGE(TAG, "Scan start failed: %s", esp_err_to_name(err));
      }
      else
      {
        ESP_LOGI(TAG, "Start scanning...");
      }
      break;

    case ESP_GAP_BLE_SCAN_RESULT_EVT:

      esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;

      if ((scan_result->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT)
          && (esp_ble_is_ibeacon_packet(scan_result->scan_rst.ble_adv,
                                        scan_result->scan_rst.adv_data_len)))
      {
        memcpy(s_beacon_data_cb.u8_beacon_addr, scan_result->scan_rst.bda, 6);
        s_beacon_data_cb.rssi = (int8_t)scan_result->scan_rst.rssi;

        xQueueSend(s_ble_ibeacon_data.s_queue_ble_inf,
                   (void *)&s_beacon_data_cb,
                   (TickType_t)0);
      }

      break;

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