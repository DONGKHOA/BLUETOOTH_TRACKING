#ifndef SPI_H_
#define SPI_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
  SPI_CLOCK_100KHz = 100000,
  SPI_CLOCK_400KHZ = 
}spi_config_clock_t;

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/

  void BSP_spiDriverInit(spi_host_device_t e_spi_host);

  void BSP_spiDMADriverInit(spi_host_device_t e_spi_host,
                            spi_common_dma_t  i_dma_channel);

  void BSP_spiConfigDefault(void);

  void BSP_spiConfigMode(uint8_t u8_spi_mode);

  void BSP_spiConfigIO(gpio_num_t e_miso_io,
                       gpio_num_t e_mosi_io,
                       gpio_num_t e_sclk_io,
                       gpio_num_t e_cs_io);

  void BSP_spiMaxTransferSize(int i_max_transfer_sz);

  void BSP_spiClockSpeed(int i_clock_speed_hz);

  void BSP_spiWriteBuffer(uint8_t *u8_tx_data, size_t i64_length);

  void BSP_spiWriteByte(uint8_t u8_data);

  uint8_t BSP_spiReadByte(void);

  void BSP_spiSelectDevice(gpio_num_t e_gpio_pin);

  void BSP_spiDeselectDevice(gpio_num_t e_gpio_pin);

  void BSP_spiSChangeClock(spi_host_device_t e_spi_host, int i_clock_speed_hz);

#ifdef __cplusplus
}
#endif

#endif /* SPI_H_ */