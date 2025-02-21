/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "i2c.h"

/*****************************************************************************
 *   PRIVATE VARIABLES
 *****************************************************************************/

static i2c_config_t config         = { 0 };
static uint8_t      is_initialized = 1;

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

esp_err_t
BSP_i2cDriverInit (i2c_port_t e_i2c_num)
{
  printf("%d", is_initialized);
  switch (e_i2c_num)
  {
    case 0:
      i2c_param_config(e_i2c_num, &config);
      return i2c_driver_install(e_i2c_num, config.mode, 0, 0, 0);
      is_initialized = 0;
      break;

    case 1:
      i2c_param_config(e_i2c_num, &config);
      return i2c_driver_install(e_i2c_num, config.mode, 0, 0, 0);
      is_initialized = 0;
      break;

    default:
      return ESP_OK;
      break;
  }
}

esp_err_t
BSP_i2cConfigMode (i2c_port_t e_i2c_num, i2c_mode_t e_i2c_mode)
{
  config.mode = e_i2c_mode;
  return ESP_OK;
}

esp_err_t
BSP_i2cConfigSDA (i2c_port_t    e_i2c_num,
                  gpio_num_t    e_sda_io,
                  gpio_pullup_t e_sda_pullup)
{
  config.sda_io_num    = e_sda_io;
  config.sda_pullup_en = e_sda_pullup;

  return ESP_OK;
}

esp_err_t
BSP_i2cConfigSCL (i2c_port_t    e_i2c_num,
                  gpio_num_t    e_scl_io,
                  gpio_pullup_t e_scl_pullup)
{
  config.scl_io_num    = e_scl_io;
  config.scl_pullup_en = e_scl_pullup;
  return ESP_OK;
}

esp_err_t
BSP_i2cConfigClockSpeed (i2c_port_t e_i2c_num, uint32_t u32_clk_speed)
{
  config.master.clk_speed = u32_clk_speed;
  return ESP_OK;
}

esp_err_t
BSP_i2cWriteBuffer (i2c_port_t e_i2c_port,
                    uint8_t    u8_device_address,
                    uint8_t   *p_data,
                    uint32_t   u32_data_length,
                    TickType_t u32_timeout)
{
  return i2c_master_write_to_device(
      e_i2c_port, u8_device_address, p_data, sizeof(p_data), u32_timeout);
}

esp_err_t
BSP_i2cReadBuffer (i2c_port_t e_i2c_port,
                   uint8_t    u8_device_address,
                   uint8_t   *p_data,
                   uint32_t   u32_data_length,
                   TickType_t u32_timeout)
{
  return i2c_master_read_from_device(
      e_i2c_port, u8_device_address, p_data, u32_data_length, u32_timeout);
}

esp_err_t
BSP_i2cWriteReadBuffer (i2c_port_t e_i2c_port,
                        uint8_t    u8_device_address,
                        uint8_t   *p_tx_data,
                        uint32_t   u32_tx_data_length,
                        uint8_t   *p_rx_data,
                        uint32_t   u32_rx_data_length,
                        TickType_t u32_timeout)
{
  return i2c_master_write_read_device(e_i2c_port,
                                      u8_device_address,
                                      p_tx_data,
                                      u32_tx_data_length,
                                      p_rx_data,
                                      u32_rx_data_length,
                                      u32_timeout);
}
