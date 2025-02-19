/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"

#include "gpio.h"

#include "spi.h"

/*****************************************************************************
 *   PRIVATE VARIABLES
 *****************************************************************************/

spi_device_handle_t spi_handle = NULL;

spi_bus_config_t buscfg = { 0 };

spi_device_interface_config_t devcfg = { 0 };

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/
void
BSP_spiDriverInit (spi_host_device_t e_spi_host)
{
  spi_bus_initialize(e_spi_host, &buscfg, SPI_DMA_DISABLED);
  spi_bus_add_device(e_spi_host, &devcfg, &spi_handle);
}

void
BSP_spiDMADriverInit (spi_host_device_t e_spi_host,
                      spi_common_dma_t  e_dma_channel)
{
  spi_bus_initialize(e_spi_host, &buscfg, e_dma_channel);
  spi_bus_add_device(e_spi_host, &devcfg, &spi_handle);
}

void
BSP_spiConfigDefault (void)
{
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
}

void
BSP_spiConfigMode (uint8_t u8_spi_mode)
{
  devcfg.mode = u8_spi_mode;
}

void
BSP_spiConfigIO (gpio_num_t e_miso_io,
                 gpio_num_t e_mosi_io,
                 gpio_num_t e_sclk_io,
                 gpio_num_t e_cs_io)
{
  buscfg.miso_io_num = e_miso_io;
  buscfg.mosi_io_num = e_mosi_io;
  buscfg.sclk_io_num = e_sclk_io;

  devcfg.spics_io_num = e_cs_io;
}

void
BSP_spiMaxTransferSize (int i_max_transfer_sz)
{
  buscfg.max_transfer_sz = i_max_transfer_sz;
}

void
BSP_spiClockSpeed (int i_clock_speed_hz)
{
  devcfg.clock_speed_hz = i_clock_speed_hz;
}

void
BSP_spiTransactionQueueSize (uint32_t u32_queue_size)
{
  devcfg.queue_size = u32_queue_size;
}

void
BSP_spiWriteBuffer (uint8_t *u8_tx_data, uint32_t u32_length)
{
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length    = u32_length * 8;
  t.tx_buffer = u8_tx_data;
  spi_device_polling_transmit(spi_handle, &t);
}

void
BSP_spiWriteByte (uint8_t data)
{
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length    = 8;
  t.tx_buffer = &data;
  t.rx_buffer = NULL;
  spi_device_polling_transmit(spi_handle, &t);
}

uint8_t
BSP_spiReadByte (void)
{
  uint8_t           txData = 0xFF, rxData = 0xFF;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length    = 8;
  t.tx_buffer = &txData;
  t.rx_buffer = &rxData;
  spi_device_polling_transmit(spi_handle, &t);
  return rxData;
}

void
BSP_spiSelectDevice (gpio_num_t e_gpio_pin)
{
  BSP_gpioSetState(e_gpio_pin, 0);
}

void
BSP_spiDeselectDevice (gpio_num_t e_gpio_pin)
{
  BSP_gpioSetState(e_gpio_pin, 1);
}

void
BSP_spiSChangeClock (spi_host_device_t e_spi_host, int i_clock_speed_hz)
{
  spi_device_interface_config_t devcfg
      = { .clock_speed_hz = i_clock_speed_hz, .queue_size = 50 };

  spi_bus_add_device(e_spi_host, &devcfg, &spi_handle);
}