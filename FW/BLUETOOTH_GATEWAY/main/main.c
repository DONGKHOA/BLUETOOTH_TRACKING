/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

/*** standard ****************************************************************/

#include <string.h>

/*** esp - idf ***************************************************************/

/*** bsp *********************************************************************/

#include "gpio.h"
#include "exti.h"
#include "nvs_rw.h"

/*** device ******************************************************************/

/*** application *************************************************************/

#include "app_data.h"
#include "app_ble_ibeacon.h"
#include "app_ble_tracking.h"
#include "app_mqtt_client.h"
#include "app_handle_wifi.h"
#include "app_led_status.h"
#include "app_timestamp.h"
#include "app_configuration.h"
#include "app_user_button.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG             "APP_MAIN"
#define LOADING_TIMEOUT 3000 // ms

/******************************************************************************
 *   PUBLIC DATA
 *****************************************************************************/

static TimerHandle_t s_loading_timer;

/******************************************************************************
 *    PRIVATE VARIABLES
 *****************************************************************************/

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static inline void APP_MAIN_InitGPIO(void);
static inline void APP_MAIN_InitEXTI(void);
static inline void APP_MAIN_InitNVS(void);
static inline void APP_MAIN_InitDataSystem(void);
static void        APP_MAIN_Loading_Callback(TimerHandle_t xTimer);

/******************************************************************************
 *       MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  // BSP Initialization

  APP_MAIN_InitGPIO();
  APP_MAIN_InitEXTI();
  APP_MAIN_InitNVS();

  // Main Initialization data system

  APP_MAIN_InitDataSystem();

  // App Initialization

  xTimerStart(s_loading_timer, 0);

  while (1)
  {
    EventBits_t uxBits = xEventGroupWaitBits(
        s_data_system.s_configuration_event,
        APP_CONFIGURATION_ENABLE | APP_CONFIGURATION_DISABLE,
        pdTRUE,
        pdFALSE,
        portMAX_DELAY);
    if (uxBits & APP_CONFIGURATION_ENABLE)
    {
      APP_CONFIGURATION_Init();

      APP_CONFIGURATION_CreateTask();
      break;
    }
    else if (uxBits & APP_CONFIGURATION_DISABLE)
    {
      APP_BLE_IBEACON_Init();
      APP_BLE_TRACKING_Init();
      APP_HANDLE_WIFI_Init();
      APP_MQTT_CLIENT_Init();

      APP_HANDLE_WIFI_CreateTask();
      APP_BLE_IBEACON_CreateTask();
      APP_BLE_TRACKING_CreateTask();
      APP_MQTT_CLIENT_CreateTask();
      break;
    }
  }
}

/******************************************************************************
 *       PRIVATE FUNCTION
 *****************************************************************************/

static inline void
APP_MAIN_InitGPIO (void)
{
  BSP_gpioSetDirection(LED_STATUS_PIN, GPIO_MODE_OUTPUT, GPIO_NO_PULL);
}

static inline void
APP_MAIN_InitEXTI (void)
{
  BSP_EXIT_Init(BUTTON_USER_PIN, EXTI_EDGE_ALL, GPIO_PULL_UP);
}

static inline void
APP_MAIN_InitNVS (void)
{
  NVS_Init();
}

static inline void
APP_MAIN_InitDataSystem (void)
{
  s_data_system.s_configuration_event = xEventGroupCreate();

  s_loading_timer = xTimerCreate("loading_timer",
                                 LOADING_TIMEOUT / portTICK_PERIOD_MS,
                                 pdFALSE,
                                 (void *)0,
                                 APP_MAIN_Loading_Callback);

  s_data_system.s_rssi_ibeacon_queue
      = xQueueCreate(16, sizeof(ibeacon_infor_tag_t));
  s_data_system.s_location_tag_queue
      = xQueueCreate(16, sizeof(tracking_infor_tag_t));
}

static void
APP_MAIN_Loading_Callback (TimerHandle_t xTimer)
{
  xTimerStop(s_loading_timer, 0);
  if (BSP_gpioGetState(BUTTON_USER_PIN) == 1)
  {
    xEventGroupSetBits(s_data_system.s_configuration_event,
                       APP_CONFIGURATION_DISABLE);
  }
  else
  {
    xEventGroupSetBits(s_data_system.s_configuration_event,
                       APP_CONFIGURATION_ENABLE);
  }
}