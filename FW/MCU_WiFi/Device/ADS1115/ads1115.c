/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "esp_log.h"

#include "ads1115.h"

/******************************************************************************
 *      PRIVATE DEFINES
 *****************************************************************************/

#define I2C_NUM        0
#define I2C_MODE       I2C_MODE_MASTER
#define I2C_SDA        GPIO_NUM_21
#define I2C_SCL        GPIO_NUM_22
#define I2C_SDA_PULLUP GPIO_PULLUP_ENABLE
#define I2C_SCL_PULLUP GPIO_PULLUP_ENABLE
#define I2C_CLK_SPEED  100000

#define ADS1115_RESOLUTION 16                              // bit
#define ADS1115_STEP       (1 << (ADS1115_RESOLUTION - 1)) // 32768

/*****************************************************************************
 *      PRIVATE VARIABLES
 *****************************************************************************/

static uint8_t i2c_buffer[3];

static const char *TAG = "ADS1115";

/*****************************************************************************
 *      PRIVATE PROTOTYPE FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 *      PUBLIC FUNCTIONS
 *****************************************************************************/
void
DEV_ADS1115_Init (void)
{
  BSP_i2cDriverInit(I2C_NUM,
                    I2C_MODE,
                    I2C_SDA,
                    I2C_SCL,
                    I2C_SDA_PULLUP,
                    I2C_SCL_PULLUP,
                    I2C_CLK_SPEED);
}
uint16_t
DEV_ADS1115_GetData (ads1115_channel_t e_channel, ads1115_gain_t e_gain)
{
  // Start with default values
  uint16_t config
      = ADS1115_REG_CONFIG_CQUE_NONE |   // Disable the comparator (default val)
        ADS1115_REG_CONFIG_CLAT_NONLAT | // Non-latching (default val)
        ADS1115_REG_CONFIG_CPOL_ACTVLOW
        |                               // Alert/Rdy active low   (default val)
        ADS1115_REG_CONFIG_CMODE_TRAD | // Traditional comparator (default val)
        ADS1115_REG_CONFIG_DR_860SPS |  // 128 samples per second (default)
        ADS1115_REG_CONFIG_MODE_SINGLE; // Single-shot mode (default)

  // Set PGA/voltage range
  config |= e_gain;
  switch (e_channel)
  {
    case DEV_ADS1115_CHANNEL_0:
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_0;
      break;
    case DEV_ADS1115_CHANNEL_1:
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_1;
      break;
    case DEV_ADS1115_CHANNEL_2:
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_2;
      break;
    case DEV_ADS1115_CHANNEL_3:
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_3;
      break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;
  i2c_buffer[0] = ADS1115_REG_POINTER_CONFIG;
  i2c_buffer[1] = (uint8_t)(config >> 8);
  i2c_buffer[2] = (uint8_t)(config & 0xFF);

  BSP_i2cWriteToDevice(I2C_NUM, ADS_ADDR_GND, i2c_buffer, 3, 1000);

  // Wait for the conversion to complete
  vTaskDelay(ADS1115_CONVERSIONDELAY_8 / portTICK_PERIOD_MS);

  // Read the conversion results
  uint8_t reg         = ADS1115_REG_POINTER_CONVERT;
  uint8_t adc_data[2] = { 0 };

  BSP_i2cWriteReadToDevice(I2C_NUM, ADS_ADDR_GND, &reg, 1, adc_data, 2, 1000);

  // Convert the results
  uint16_t result = (adc_data[0] << 8) | adc_data[1];

  ESP_LOGI(TAG, "ADC Value: %d", result);
  return result;
}
float
DEV_ADS1115_ReadVoltage (ads1115_channel_t e_channel, ads1115_gain_t e_gain)
{
  // Read raw Data
  uint16_t value = DEV_ADS1115_GetData(e_channel, e_gain);

  // Calib ADC

  // Convert voltage
  return (float)(value * ADS1115_VREF) / ADS1115_STEP;
}
float
DEV_ADS1115_GetVoltage (uint16_t u16_value_adc)
{
  // Convert voltage
  float voltage = (float)(u16_value_adc * ADS1115_VREF) / ADS1115_STEP;
  return voltage;
}