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

/*** GPIO peripheral *********************************************************/

#define BUTTON_USER_PIN GPIO_NUM_12
#define LED_STATUS_PIN  GPIO_NUM_5

/******************************************************************************
 *   PUBLIC DATA
 *****************************************************************************/

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

  APP_USER_BUTTON_Init();
  APP_CONFIGURATION_Init();
  // APP_BLE_IBEACON_Init();
  // APP_BLE_TRACKING_Init();
  // APP_HANDLE_WIFI_Init();
  // // APP_TIMESTAMP_Init();
  // APP_MQTT_CLIENT_Init();

  // App Create Task

  // APP_BLE_IBEACON_CreateTask();
  // APP_BLE_TRACKING_CreateTask();
  // // APP_HANDLE_WIFI_CreateTask();
  // // APP_TIMESTAMP_CreateTask();
  // APP_MQTT_CLIENT_CreateTask();
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
  memset(s_data_system.u8_ssid, 0, sizeof(s_data_system.u8_ssid));
  memset(s_data_system.u8_pass, 0, sizeof(s_data_system.u8_pass));

  s_data_system.s_configuration_event = xEventGroupCreate();

  s_data_system.s_rssi_ibeacon_queue
      = xQueueCreate(16, sizeof(ibeacon_infor_tag_t));
  s_data_system.s_location_tag_queue
      = xQueueCreate(16, sizeof(tracking_infor_tag_t));
}
