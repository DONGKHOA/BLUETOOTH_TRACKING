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

#include "environment.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

/*** CAN peripheral **********************************************************/

#define CAN_MODE          TWAI_MODE_NORMAL
#define CAN_TXD_PIN       GPIO_NUM_42
#define CAN_RXD_PIN       GPIO_NUM_41
#define CAN_TXD_QUEUE_LEN 8
#define CAN_RXD_QUEUE_LEN 8
#define CAN_INTR_FLAG     ESP_INTR_FLAG_LEVEL3 // lowest priority
#define CAN_BITRATE       5

/*** I2C peripheral **********************************************************/
#define I2C_NUM        0
#define I2C_MODE       I2C_MODE_MASTER
#define I2C_SDA        GPIO_NUM_39
#define I2C_SCL        GPIO_NUM_40
#define I2C_SDA_PULLUP GPIO_PULLUP_ENABLE
#define I2C_SCL_PULLUP GPIO_PULLUP_ENABLE
#define I2C_CLK_SPEED  100000

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static inline void APP_MAIN_InitGPIO(void);
static inline void APP_MAIN_InitCan(void);
static inline void APP_MAIN_InitI2C(void);
static inline void APP_MAIN_InitNVS(void);
static inline void APP_MAIN_InitUart(void);
static inline void APP_MAIN_InitDataSystem(void);

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

  // APP_FINGERPRINT_Init();
  // APP_DATA_TRANSMIT_Init();
  APP_HANDLE_WIFI_Init();
  APP_TIMESTAMP_Init();
  APP_RTC_Init();

  // App Create Task

  // APP_FINGERPRINT_CreateTask();
  // APP_DATA_TRANSMIT_CreateTask();
  APP_HANDLE_WIFI_CreateTask();
  APP_TIMESTAMP_CreateTask();
  // APP_RTC_CreateTask();
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static inline void
APP_MAIN_InitGPIO (void)
{
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
  s_data_system.s_send_data_queue = xQueueCreate(8, sizeof(DATA_SYNC_t));
  s_data_system.s_flag_time_event = xEventGroupCreate();
}