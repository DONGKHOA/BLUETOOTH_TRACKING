/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <string.h>

#include "nvs_rw.h"
#include "app_mqtt_client.h"
#include "app_data.h"

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static inline void APP_MAIN_InitDataSystem(void)
{
  memset(s_data_system.u8_ssid, 0, sizeof(s_data_system.u8_ssid));
  memset(s_data_system.u8_pass, 0, sizeof(s_data_system.u8_pass));

  s_data_system.s_data_mqtt_queue = xQueueCreate(32, sizeof(uint32_t));
}

/******************************************************************************
 *       MAIN FUNCTION
 *****************************************************************************/

void app_main(void)
{
  // BSP Initialization
  NVS_Init();

  // Main Initialization data system
  APP_MAIN_InitDataSystem();

  // App Initialization
  APP_MQTT_CLIENT_Init();

  // App Create Task
  APP_MQTT_CLIENT_CreateTask();
}
