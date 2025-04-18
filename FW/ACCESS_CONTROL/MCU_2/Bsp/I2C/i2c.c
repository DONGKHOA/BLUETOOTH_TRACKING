/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "i2c.h"

/*****************************************************************************
 *   PRIVATE VARIABLES
 *****************************************************************************/

static i2c_config_t config = { 0 };

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

esp_err_t
BSP_i2cDriverInit (i2c_port_t e_i2c_num)
{
  i2c_param_config(e_i2c_num, &config);
  return i2c_driver_install(e_i2c_num, config.mode, 0, 0, 0);
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