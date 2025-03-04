#ifndef I2C_H_
#define I2C_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/gpio.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/

  esp_err_t BSP_i2cDriverInit(i2c_port_t e_i2c_num);

  esp_err_t BSP_i2cConfigMode(i2c_port_t e_i2c_num, i2c_mode_t e_i2c_mode);

  esp_err_t BSP_i2cConfigSDA(i2c_port_t    e_i2c_num,
                             gpio_num_t    e_sda_io,
                             gpio_pullup_t e_sda_pullup);

  esp_err_t BSP_i2cConfigSCL(i2c_port_t    e_i2c_num,
                             gpio_num_t    e_scl_io,
                             gpio_pullup_t e_scl_pullup);

  esp_err_t BSP_i2cConfigClockSpeed(i2c_port_t e_i2c_num,
                                    uint32_t   u32_clk_speed);

  /**
   * @brief Write data to an I2C device.
   *
   * This function writes a specified amount of data to a device on the I2C
   * bus.
   *
   * @param e_i2c_port       The I2C port number to use.
   * @param u8_device_address The address of the I2C device to write to.
   * @param p_data           Pointer to the data buffer to write.
   * @param u32_data_length  The length of the data to write.
   * @param u32_timeout      The timeout for the I2C operation.
   *
   * @return
   *     - ESP_OK: Success
   *     - ESP_ERR_INVALID_ARG: Parameter error
   *     - ESP_FAIL: Sending command error, slave doesn't ACK the transfer.
   *     - ESP_ERR_INVALID_STATE: I2C driver not installed or not in master
   * mode.
   *     - ESP_ERR_TIMEOUT: Operation timeout because the bus is busy.
   */
  esp_err_t BSP_i2cWriteBuffer(i2c_port_t e_i2c_port,
                               uint8_t    u8_device_address,
                               uint8_t   *p_data,
                               uint32_t   u32_data_length,
                               TickType_t u32_timeout);
  /**
   * @brief Read data from an I2C device.
   *
   * This function reads a specified number of bytes from an I2C device.
   *
   * @param e_i2c_port       I2C port number to use for the operation.
   * @param u8_device_address I2C address of the target device.
   * @param p_data           Pointer to the buffer where the read data will be
   * stored.
   * @param u32_data_length  Number of bytes to read from the device.
   * @param u32_timeout      Maximum time to wait for the read operation to
   * complete.
   *
   * @return
   *     - ESP_OK: Success
   *     - ESP_ERR_INVALID_ARG: Parameter error
   *     - ESP_FAIL: Sending command error, slave doesn't ACK the transfer.
   *     - ESP_ERR_INVALID_STATE: I2C driver not installed or not in master
   * mode.
   *     - ESP_ERR_TIMEOUT: Operation timeout because the bus is busy.
   */
  esp_err_t BSP_i2cReadBuffer(i2c_port_t e_i2c_port,
                              uint8_t    u8_device_address,
                              uint8_t   *p_data,
                              uint32_t   u32_data_length,
                              TickType_t u32_timeout);

  /**
   * @brief Perform an I2C write-read operation to a specified device.
   *
   * This function writes data to an I2C device and then reads data from it.
   *
   * @param e_i2c_port        I2C port number to use for the operation.
   * @param u8_device_address Address of the I2C device.
   * @param p_tx_data         Pointer to the data to be transmitted.
   * @param u32_tx_data_length Length of the data to be transmitted.
   * @param p_rx_data         Pointer to the buffer to store received data.
   * @param u32_rx_data_length Length of the data to be received.
   * @param u32_timeout       Timeout for the I2C operation.
   *
   * @return
   *     - ESP_OK: Success
   *     - ESP_ERR_INVALID_ARG: Parameter error
   *     - ESP_FAIL: Sending command error, slave doesn't ACK the transfer.
   *     - ESP_ERR_INVALID_STATE: I2C driver not installed or not in master
   * mode.
   *     - ESP_ERR_TIMEOUT: Operation timeout because the bus is busy.
   */
  esp_err_t BSP_i2cWriteReadBuffer(i2c_port_t e_i2c_port,
                                   uint8_t    u8_device_address,
                                   uint8_t   *p_tx_data,
                                   uint32_t   u32_tx_data_length,
                                   uint8_t   *p_rx_data,
                                   uint32_t   u32_rx_data_length,
                                   TickType_t u32_timeout);

#ifdef __cplusplus
}
#endif

#endif /* I2C_H_ */