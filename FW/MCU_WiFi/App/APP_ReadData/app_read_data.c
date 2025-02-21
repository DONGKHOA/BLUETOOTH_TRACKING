/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_read_data.h"

/******************************************************************************
 *  PRIVATE VARIABLE
 *****************************************************************************/

#define I2C_NUM        0
#define I2C_MODE       I2C_MODE_MASTER
#define I2C_SDA        GPIO_NUM_41
#define I2C_SCL        GPIO_NUM_42
#define I2C_SDA_PULLUP GPIO_PULLUP_ENABLE
#define I2C_SCL_PULLUP GPIO_PULLUP_ENABLE
#define I2C_CLK_SPEED  100000

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_ADS1115_Task(void *pvParameter);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_ReadData_CreateTask (void)
{
  xTaskCreate(APP_ADS1115_Task, "APP_ADS1115_Task", 4096, NULL, 8, NULL);
}

void
APP_ReadData_Init (void)
{
  BSP_i2cConfigMode(I2C_NUM, I2C_MODE);
  BSP_i2cConfigSDA(I2C_NUM, I2C_SDA, I2C_SDA_PULLUP);
  BSP_i2cConfigSCL(I2C_NUM, I2C_SCL, I2C_SCL_PULLUP);
  BSP_i2cConfigClockSpeed(I2C_NUM, I2C_CLK_SPEED);
  BSP_i2cDriverInit(I2C_NUM);
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_ADS1115_Task (void *pvParameter)
{
  while (1)
  {
    int16_t adc_value = DEV_ADS1115_GetData(
        I2C_NUM, DEV_ADS1115_CHANNEL_0, ADS1115_REG_CONFIG_PGA_4_096V);
    float voltage = DEV_ADS1115_GetVoltage(adc_value);

    printf("Value: %d, Voltage: %.3f V\n", adc_value, voltage);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}