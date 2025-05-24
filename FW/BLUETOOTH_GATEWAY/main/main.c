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
#include "i2c.h"

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
#include "app_display.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG             "APP_MAIN"
#define LOADING_TIMEOUT 3000 // ms

/******************************************************************************
 *   PRIVATE DATA
 *****************************************************************************/

static TimerHandle_t s_loading_timer;

/******************************************************************************
 *    PRIVATE VARIABLES
 *****************************************************************************/

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static inline void APP_MAIN_InitGPIO(void);
static inline void APP_MAIN_InitI2C(void);
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
  APP_MAIN_InitI2C();

  // Main Initialization data system

  APP_MAIN_InitDataSystem();

  // App Initialization

  xTimerStart(s_loading_timer, 0);

  APP_STATUS_LED_Init();

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
      // APP_DISPLAY_Init();

      APP_HANDLE_WIFI_CreateTask();
      APP_BLE_IBEACON_CreateTask();
      APP_BLE_TRACKING_CreateTask();
      APP_MQTT_CLIENT_CreateTask();
      // APP_DISPLAY_CreateTask();
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
APP_MAIN_InitI2C (void)
{
  BSP_i2cConfigMode(I2C_NUM, I2C_MODE);
  BSP_i2cConfigSDA(I2C_NUM, I2C_SDA, I2C_SDA_PULLUP);
  BSP_i2cConfigSCL(I2C_NUM, I2C_SCL, I2C_SCL_PULLUP);
  BSP_i2cConfigClockSpeed(I2C_NUM, I2C_CLK_SPEED);
  BSP_i2cDriverInit(I2C_NUM);
}

static inline void
APP_MAIN_InitEXTI (void)
{
  BSP_EXIT_Init(USER_BUTTON_1_PIN, EXTI_EDGE_ALL, GPIO_PULL_UP);
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

  s_data_system.s_addr_tag_queue = xQueueCreate(4, sizeof(addr_tag_t));

  s_data_system.s_mutex_num_tag = xSemaphoreCreateMutex();
}

static void
APP_MAIN_Loading_Callback (TimerHandle_t xTimer)
{
  xTimerStop(s_loading_timer, 0);
  if (BSP_gpioGetState(USER_BUTTON_1_PIN) == 1)
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