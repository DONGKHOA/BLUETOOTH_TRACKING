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
#include "string.h"

#include "ads1115.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

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

/******************************************************************************
 *    PRIVATE FUNCTIONS
 *****************************************************************************/

static void TestMain_ADS11115_Init(void);
static void TestMain_ADS1115_Task(void *pvParameter);

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{

  TestMain_ADS11115_Init();

  xTaskCreate(TestMain_ADS1115_Task, "TestMain_ADS1115_Task", 4096, NULL, 8, NULL);

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
TestMain_ADS11115_Init (void)
{
  BSP_i2cConfigMode(I2C_NUM, I2C_MODE);
  BSP_i2cConfigSDA(I2C_NUM, I2C_SDA, I2C_SDA_PULLUP);
  BSP_i2cConfigSCL(I2C_NUM, I2C_SCL, I2C_SCL_PULLUP);
  BSP_i2cConfigClockSpeed(I2C_NUM, I2C_CLK_SPEED);
  BSP_i2cDriverInit(I2C_NUM);
}

static void
TestMain_ADS1115_Task (void *pvParameter)
{
  while (1)
  {
    float voltage = DEV_ADS1115_ReadVoltage(I2C_NUM, DEV_ADS1115_CHANNEL_0, GAIN_ONE);

    printf("Voltage: %.3f V\n", voltage);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}