/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "ds3231.h"
/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define I2C_NUM        0
#define I2C_MODE       I2C_MODE_MASTER
#define I2C_SDA        GPIO_NUM_41
#define I2C_SCL        GPIO_NUM_42
#define I2C_SDA_PULLUP GPIO_PULLUP_ENABLE
#define I2C_SCL_PULLUP GPIO_PULLUP_ENABLE
#define I2C_CLK_SPEED  100000

/******************************************************************************
 *    PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static ds3231_data_t s_ds3231_data;

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void TestMain_RTC_Init(void);
static void TestMain_RTC_Task(void *pvParameter);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  TestMain_RTC_Init();

  xTaskCreate(TestMain_RTC_Task, "TestMain_RTC_Task", 4096, NULL, 9, NULL);

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
TestMain_RTC_Init (void)
{
  BSP_i2cConfigMode(I2C_NUM, I2C_MODE);
  BSP_i2cConfigSDA(I2C_NUM, I2C_SDA, I2C_SDA_PULLUP);
  BSP_i2cConfigSCL(I2C_NUM, I2C_SCL, I2C_SCL_PULLUP);
  BSP_i2cConfigClockSpeed(I2C_NUM, I2C_CLK_SPEED);
  BSP_i2cDriverInit(I2C_NUM);

  DEV_DS3231_Init(&s_ds3231_data, I2C_NUM);
}

static void
TestMain_RTC_Task (void *pvParameter)
{
  while (1)
  {
    DEV_DS3231_Register_Read(&s_ds3231_data, I2C_NUM);
    printf("Time: %02d:%02d:%02d %02d/%02d/%02d\n",
           s_ds3231_data.u8_hour,
           s_ds3231_data.u8_minute,
           s_ds3231_data.u8_second,
           s_ds3231_data.u8_date,
           s_ds3231_data.u8_month,
           s_ds3231_data.u8_year);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}