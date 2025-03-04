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
   *      PUBLIC TYPEDEFS
   *****************************************************************************/

  typedef enum
  {
    SPI_CLOCK_100KHz = 100000,
    SPI_CLOCK_400KHz = 400000,
    SPI_CLOCK_1MHz   = 1000000,
    SPI_CLOCK_5MHz   = 5000000,
    SPI_CLOCK_10MHz  = 10000000,
    SPI_CLOCK_20MHz  = 20000000,
    SPI_CLOCK_40MHz  = 40000000
  } spi_config_clock_t;

  /*****************************************************************************
   *      PUBLIC FUNCTIONS
   *****************************************************************************/

  /**
   * @brief Initialize the SPI driver for the specified host without DMA.
   *
   * @param spi_handle The SPI device handle to initialize.
   * @param e_spi_host The SPI host device to initialize.
   */
  void BSP_spiDriverInit(spi_device_handle_t *spi_handle,
                         spi_host_device_t    e_spi_host);

  /**
   * @brief Initialize the SPI DMA driver for the specified host and DMA
   * channel.
   *
   * @param spi_handle The SPI device handle to initialize.
   * @param e_spi_host The SPI host device to initialize.
   * @param e_dma_channel The DMA channel to use for SPI transactions.
   */
  void BSP_spiDMADriverInit(spi_device_handle_t *spi_handle,
                            spi_host_device_t    e_spi_host,
                            spi_common_dma_t     e_dma_channel);

  /**
   * @brief Configure the SPI with default settings.
   */
  void BSP_spiConfigDefault(void);

  /**
   * @brief Configure the SPI mode.
   *
   * @param u8_spi_mode The SPI mode to set (0-3).
   */
  void BSP_spiConfigMode(uint8_t u8_spi_mode);

  /**
   * @brief Configure the SPI IO pins.
   *
   * @param e_miso_io The GPIO pin for MISO.
   * @param e_mosi_io The GPIO pin for MOSI.
   * @param e_sclk_io The GPIO pin for SCLK.
   */
  void BSP_spiConfigIO(gpio_num_t e_miso_io,
                       gpio_num_t e_mosi_io,
                       gpio_num_t e_sclk_io);

  /**
   * @brief Configure the SPI chip select (CS) pin.
   *
   * @param e_cs_io The GPIO pin for CS.
   */
  void BSP_spiConfigCS(gpio_num_t e_cs_io);

  /**
   * @brief Set the maximum transfer size for SPI transactions.
   *
   * @param i_max_transfer_sz The maximum transfer size in bytes.
   */
  void BSP_spiMaxTransferSize(int i_max_transfer_sz);

  /**
   * @brief Set the SPI clock speed.
   *
   * @param e_clock_speed_hz The clock speed in Hz.
   */
  void BSP_spiClockSpeed(spi_config_clock_t e_clock_speed_hz);

  /**
   * @brief Set the size of the SPI transaction queue.
   *
   * @param u32_queue_size The size of the transaction queue.
   */
  void BSP_spiTransactionQueueSize(uint32_t u32_queue_size);

  /**
   * @brief Write a buffer of data to the SPI bus.
   *
   * @param spi_handle The SPI device handle to initialize.
   * @param p_tx_data Pointer to the data buffer to write.
   * @param u32_length The length of the data buffer.
   */
  void BSP_spiWriteMultipleByte(spi_device_handle_t spi_handle,
                                uint8_t            *p_tx_data,
                                uint32_t            u32_length);

  /**
   * @brief Write a single byte to the SPI bus.
   *
   * @param spi_handle The SPI device handle to initialize.
   * @param u8_data The byte to write.
   */
  void BSP_spiWriteByte(spi_device_handle_t spi_handle, uint8_t u8_data);

  /**
   * @brief Read a single byte from the SPI bus.
   *
   * @param spi_handle The SPI device handle to initialize.
   *
   * @return The byte read from the SPI bus.
   */
  uint8_t BSP_spiReadByte(spi_device_handle_t spi_handle);

  /**
   * @brief Select the SPI device by setting the CS pin low.
   *
   * @param e_gpio_pin The GPIO pin for CS.
   */
  static inline void BSP_spiSelectDevice (gpio_num_t e_gpio_pin)
  {
    BSP_gpioSetState(e_gpio_pin, 0);
  }
  /**
   * @brief Deselect the SPI device by setting the CS pin high.
   *
   * @param e_gpio_pin The GPIO pin for CS.
   */
  static inline void BSP_spiDeselectDevice (gpio_num_t e_gpio_pin)
  {
    BSP_gpioSetState(e_gpio_pin, 1);
  }

  /**
   * @brief Change the SPI clock speed for the specified host.
   *
   * @param spi_handle The SPI device handle to initialize.
   * @param e_spi_host The SPI host device.
   * @param e_clock_speed_hz The new clock speed in Hz.
   *
   * @return esp_err_t ESP_OK on success, or an error code on failure.
   */
  esp_err_t BSP_spiSChangeClock(spi_device_handle_t spi_handle,
                                spi_host_device_t   e_spi_host,
                                spi_config_clock_t  e_clock_speed_hz);
#ifdef __cplusplus
}
#endif

#endif /* SPI_H_ */