/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

/*** standard ****************************************************************/

#include <string.h>

/*** esp - idf ***************************************************************/

/*** bsp *********************************************************************/

#include "can.h"
#include "spi.h"
#include "gpio.h"
#include "i2c.h"
#include "nvs_rw.h"
#include "uart.h"

/*** device ******************************************************************/

/*** application *************************************************************/

#include "app_data.h"
#include "app_mqtt_client.h"
#include "app_data_receive.h"
#include "app_data_transmit.h"
#include "app_handle_wifi.h"
#include "app_fingerprint.h"
#include "app_timestamp.h"
#include "app_rtc.h"
#include "app_configuration.h"

#include "environment.h"

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
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static inline void APP_MAIN_InitGPIO(void);
static inline void APP_MAIN_InitCan(void);
static inline void APP_MAIN_InitI2C(void);
static inline void APP_MAIN_InitNVS(void);
static inline void APP_MAIN_InitUart(void);
static inline void APP_MAIN_InitDataSystem(void);
static void        APP_MAIN_Loading_Callback(TimerHandle_t xTimer);

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  // BSP Initialization
  APP_MAIN_InitCan();
  APP_MAIN_InitNVS();
  APP_MAIN_InitUart();
  APP_MAIN_InitI2C();

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
      // APP_FINGERPRINT_Init();
      APP_DATA_TRANSMIT_Init();
      APP_DATA_RECEIVE_Init();
      APP_HANDLE_WIFI_Init();
      // APP_TIMESTAMP_Init();
      // APP_RTC_Init();
      APP_MQTT_CLIENT_Init();

      // App Create Task

      // APP_FINGERPRINT_CreateTask();
      APP_DATA_TRANSMIT_CreateTask();
      APP_DATA_RECEIVE_CreateTask();
      APP_HANDLE_WIFI_CreateTask();
      // APP_TIMESTAMP_CreateTask();
      APP_MQTT_CLIENT_CreateTask();
      // APP_RTC_CreateTask();
      break;
    }
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static inline void
APP_MAIN_InitGPIO (void)
{
  BSP_gpioSetDirection(LED_STATUS_PIN, GPIO_MODE_OUTPUT, GPIO_NO_PULL);
  BSP_gpioSetDirection(BUTTON_USER_PIN, GPIO_MODE_INPUT, GPIO_PULLUP_ENABLE);
}

static inline void
APP_MAIN_InitCan (void)
{
  // Config parameter of CAN protocol
  BSP_canConfigDefault();
  BSP_canConfigMode(CAN_MODE);
  BSP_canConfigIO(CAN_TXD_PIN, CAN_RXD_PIN);
  BSP_canConfigQueue(CAN_TXD_QUEUE_LEN, CAN_RXD_QUEUE_LEN);
  BSP_canConfigIntr(CAN_INTR_FLAG);
  BSP_canConfigBitRate(CAN_BITRATE);

  // Install TWAI driver
  BSP_canDriverInit();

  // Start TWAI driver
  BSP_canStart();
}

static inline void
APP_MAIN_InitNVS (void)
{
  NVS_Init();
}

static inline void
APP_MAIN_InitUart (void)
{
  BSP_uartConfigIO(UART_NUM, UART_TXD, UART_RXD);
  BSP_uartConfigParity(UART_PARITY_DISABLE);
  BSP_uartConfigDataLen(UART_DATA_8_BITS);
  BSP_uartConfigStopBits(UART_STOP_BITS_1);
  BSP_uartConfigBaudrate(BAUD_RATE);
  BSP_uartConfigHWFlowCTRL(UART_HW_FLOWCTRL_DISABLE);

  BSP_uartDriverInit(UART_NUM);
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
APP_MAIN_InitDataSystem (void)
{
  s_data_system.s_send_data_queue = xQueueCreate(16, sizeof(DATA_SYNC_t));
  s_data_system.s_data_mqtt_queue = xQueueCreate(2, sizeof(DATA_SYNC_t));
  s_data_system.s_data_local_database_queue
      = xQueueCreate(2, sizeof(DATA_SYNC_t));
  s_data_system.s_flag_time_event     = xEventGroupCreate();
  s_data_system.s_configuration_event = xEventGroupCreate();

  s_loading_timer = xTimerCreate("loading_timer",
                                 LOADING_TIMEOUT / portTICK_PERIOD_MS,
                                 pdFALSE,
                                 (void *)0,
                                 APP_MAIN_Loading_Callback);
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