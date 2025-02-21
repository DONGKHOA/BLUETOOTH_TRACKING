/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_rtc.h"

/******************************************************************************
 *  PRIVATE DEFINES
 *****************************************************************************/

#define I2C_NUM        0
#define I2C_MODE       I2C_MODE_MASTER
#define I2C_SDA        GPIO_NUM_41 
#define I2C_SCL        GPIO_NUM_42
#define I2C_SDA_PULLUP GPIO_PULLUP_ENABLE
#define I2C_SCL_PULLUP GPIO_PULLUP_ENABLE
#define I2C_CLK_SPEED  100000

/******************************************************************************
 *  PRIVATE VARIABLES
 *****************************************************************************/

static ds3231_data_t s_ds3231_data;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTIONS
 *****************************************************************************/

static void APP_RTC_Task(void *pvParameter);

/******************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

void
APP_RTC_CreateTask (void)
{
  xTaskCreate(APP_RTC_Task, "APP_RTC_Task", 4096, NULL, 9, NULL);
}

void
APP_RTC_Init (void)
{
  BSP_i2cConfigMode(I2C_NUM, I2C_MODE);
  BSP_i2cConfigSDA(I2C_NUM, I2C_SDA, I2C_SDA_PULLUP);
  BSP_i2cConfigSCL(I2C_NUM, I2C_SCL, I2C_SCL_PULLUP);
  BSP_i2cConfigClockSpeed(I2C_NUM, I2C_CLK_SPEED);
  BSP_i2cDriverInit(I2C_NUM);

  DEV_DS3231_Init(&s_ds3231_data, I2C_NUM);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_RTC_Task (void *pvParameter)
{
  while (1)
  {
    DEV_DS3231_Register_Read(&s_ds3231_data, I2C_NUM);
    printf("Time: %02d:%02d:%02d\n",
           s_ds3231_data.u8_hour,
           s_ds3231_data.u8_minute,
           s_ds3231_data.u8_second);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}