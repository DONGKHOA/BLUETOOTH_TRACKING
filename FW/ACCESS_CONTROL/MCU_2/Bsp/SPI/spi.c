/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#include "spi.h"

/*****************************************************************************
 *   PRIVATE VARIABLES
 *****************************************************************************/

spi_bus_config_t buscfg = { 0 };

spi_device_interface_config_t devcfg = { 0 };

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/
void
BSP_spiDriverInit (spi_device_handle_t *spi_handle,
                   spi_host_device_t    e_spi_host)
{
  spi_bus_initialize(e_spi_host, &buscfg, SPI_DMA_DISABLED);
  spi_bus_add_device(e_spi_host, &devcfg, spi_handle);
}

void
BSP_spiDMADriverInit (spi_device_handle_t *spi_handle,
                      spi_host_device_t    e_spi_host,
                      spi_common_dma_t     e_dma_channel)
{
  spi_bus_initialize(e_spi_host, &buscfg, e_dma_channel);
  spi_bus_add_device(e_spi_host, &devcfg, spi_handle);
}

void
BSP_spiConfigDefault (void)
{
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;

  devcfg.flags      = 0; // Đảm bảo không bật DUMMY
  devcfg.dummy_bits = 0;
}

void
BSP_spiConfigMode (uint8_t u8_spi_mode)
{
  devcfg.mode = u8_spi_mode;
}

void
BSP_spiConfigIO (gpio_num_t e_miso_io,
                 gpio_num_t e_mosi_io,
                 gpio_num_t e_sclk_io)
{
  buscfg.miso_io_num = e_miso_io;
  buscfg.mosi_io_num = e_mosi_io;
  buscfg.sclk_io_num = e_sclk_io;
}

void
BSP_spiConfigCS (gpio_num_t e_cs_io)
{
  devcfg.spics_io_num = e_cs_io;
}

void
BSP_spiMaxTransferSize (int i_max_transfer_sz)
{
  buscfg.max_transfer_sz = i_max_transfer_sz;
}

void
BSP_spiClockSpeed (spi_config_clock_t e_clock_speed_hz)
{
  devcfg.clock_speed_hz = e_clock_speed_hz;
}

void
BSP_spiTransactionQueueSize (uint32_t u32_queue_size)
{
  devcfg.queue_size = u32_queue_size;
}

void
BSP_spiWriteMultipleByte (spi_device_handle_t spi_handle,
                          uint8_t            *p_tx_data,
                          uint32_t            u32_length)
{
  if (p_tx_data == NULL || u32_length == 0)
  {
    printf("Error: Invalid tx data pointer or length\n");
    return;
  }

  spi_transaction_t t;
  memset(&t, 0, sizeof(spi_transaction_t));
  t.length    = u32_length * 8; // Convert byte length to bit length
  t.tx_buffer = p_tx_data;
  t.flags     = 0; // Ensure SPI_TRANS_USE_TXDATA is NOT set

  esp_err_t ret = spi_device_transmit(spi_handle, &t);
  if (ret != ESP_OK)
  {
    printf("SPI transmit failed: %s\n", esp_err_to_name(ret));
  }
}

void
BSP_spiWriteByte (spi_device_handle_t spi_handle, uint8_t data)
{
  spi_transaction_t t;
  memset(&t, 0, sizeof(spi_transaction_t));
  t.length    = 8;
  t.tx_buffer = &data;
  spi_device_transmit(spi_handle, &t);
}

uint8_t
BSP_spiReadByte (spi_device_handle_t spi_handle)
{
  uint8_t           txData = 0xFF, rxData = 0xFF;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length    = 8;
  t.tx_buffer = &txData;
  t.rx_buffer = &rxData;
  spi_device_transmit(spi_handle, &t);
  return rxData;
}

esp_err_t
BSP_spiSChangeClock (spi_device_handle_t spi_handle,
                     spi_host_device_t   e_spi_host,
                     spi_config_clock_t  e_clock_speed_hz)
{
  spi_bus_remove_device(spi_handle);

  devcfg.clock_speed_hz = e_clock_speed_hz;

  return spi_bus_add_device(e_spi_host, &devcfg, &spi_handle);
}