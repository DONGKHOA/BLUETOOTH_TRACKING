#ifndef SPI_H_
#define SPI_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/

  void BSP_spiDriverInit(spi_host_device_t e_spi_host,
                         gpio_num_t        e_miso_io,
                         gpio_num_t        e_mosi_io,
                         gpio_num_t        e_sclk_io,
                         int               i_max_transfer_sz,
                         gpio_num_t        e_cs_io,
                         int               i_dma_channel,
                         int               i_clock_speed_hz,
                         uint8_t           u8_spi_mode);

  void BSP_spiWriteToDevice(uint8_t *u8_tx_data, size_t i64_length);

  void BSP_spiWriteByte(uint8_t u8_data);

  void BSP_spiReadFromDevice(uint8_t *u8_rx_data, size_t i64_length);

  uint8_t BSP_spiReadByte(void);

  void BSP_spiSelectDevice(gpio_num_t e_gpio_pin);

  void BSP_spiDeselectDevice(gpio_num_t e_gpio_pin);

  void BSP_spiSChangeClock(spi_host_device_t e_spi_host, int i_clock_speed_hz);

#ifdef __cplusplus
}
#endif

#endif /* SPI_H_ */