/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"

#include "i2c.h"

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

esp_err_t
BSP_i2cDriverInit (i2c_port_t    e_i2c_num,
                   i2c_mode_t    e_i2c_mode,
                   gpio_num_t    e_sda_io,
                   gpio_num_t    e_scl_io,
                   gpio_pullup_t e_sda_pullup,
                   gpio_pullup_t e_scl_pullup,
                   uint32_t      u32_clk_speed)
{
  uint8_t u8_i2c_master_port = e_i2c_num;

  i2c_config_t conf = {
    .mode             = e_i2c_mode,
    .sda_io_num       = e_sda_io,
    .scl_io_num       = e_scl_io,
    .sda_pullup_en    = e_sda_pullup,
    .scl_pullup_en    = e_scl_pullup,
    .master.clk_speed = u32_clk_speed,
  };

  i2c_param_config(u8_i2c_master_port, &conf);

  return i2c_driver_install(u8_i2c_master_port,
                            conf.mode,
                            I2C_MASTER_RX_BUF_DISABLE,
                            I2C_MASTER_TX_BUF_DISABLE,
                            0);
}

esp_err_t
BSP_i2cWriteToDevice (i2c_port_t e_i2c_port,
                      uint8_t    u8_device_address,
                      uint8_t   *p_data,
                      size_t     u32_data_length,
                      TickType_t u32_timeout)
{
  int ret = i2c_master_write_to_device(
      e_i2c_port, u8_device_address, p_data, sizeof(p_data), u32_timeout);
  return ret;
}

esp_err_t
BSP_i2cReadFromDevice (i2c_port_t e_i2c_port,
                       uint8_t    u8_device_address,
                       uint8_t   *p_data,
                       size_t     u32_data_length,
                       TickType_t u32_timeout)
{
  int ret = i2c_master_read_from_device(
      e_i2c_port, u8_device_address, p_data, u32_data_length, u32_timeout);
  return ret;
}

esp_err_t
BSP_i2cWriteReadToDevice (i2c_port_t e_i2c_port,
                          uint8_t    u8_device_address,
                          uint8_t   *p_tx_data,
                          size_t     u32_tx_data_length,
                          uint8_t   *p_rx_data,
                          size_t     u32_rx_data_length,
                          TickType_t u32_timeout)
{
  int ret = i2c_master_write_read_device(e_i2c_port,
                                         u8_device_address,
                                         p_tx_data,
                                         u32_tx_data_length,
                                         p_rx_data,
                                         u32_rx_data_length,
                                         u32_timeout);
  return ret;
}
