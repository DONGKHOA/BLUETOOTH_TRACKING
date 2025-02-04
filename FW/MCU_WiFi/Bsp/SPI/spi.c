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

spi_device_handle_t spi_handle = NULL;

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/
void
BSP_spiDriverInit (spi_host_device_t e_spi_host,
                   gpio_num_t        e_miso_io,
                   gpio_num_t        e_mosi_io,
                   gpio_num_t        e_sclk_io,
                   int               i_max_transfer_sz,
                   gpio_num_t        e_cs_io,
                   int               i_dma_channel,
                   int               i_clock_speed_hz,
                   uint8_t           u8_spi_mode)
{
  spi_bus_config_t buscfg = { .miso_io_num     = e_miso_io,
                              .mosi_io_num     = e_mosi_io,
                              .sclk_io_num     = e_sclk_io,
                              .quadwp_io_num   = -1,
                              .quadhd_io_num   = -1,
                              .max_transfer_sz = i_max_transfer_sz };

  spi_device_interface_config_t devcfg = { .clock_speed_hz = i_clock_speed_hz,
                                           .mode           = u8_spi_mode,
                                           .spics_io_num   = e_cs_io,
                                           .queue_size     = 50 };
  // maximum number of DMA transactions simultaneously in-flight

  spi_bus_initialize(e_spi_host, &buscfg, i_dma_channel);
  spi_bus_add_device(e_spi_host, &devcfg, &spi_handle);
}

void
BSP_spiWriteToDevice (uint8_t *u8_tx_data, size_t i64_length)
{
  spi_transaction_t master_trans
      = { .flags = 0, .tx_buffer = u8_tx_data, .length = i64_length * 8 };
  spi_device_transmit(spi_handle, &master_trans);
}

void
BSP_spiWriteByte (uint8_t u8_tx_data)
{
  BSP_spiWriteToDevice(&u8_tx_data, 1);
}

void
BSP_spiReadFromDevice (uint8_t *u8_rx_data, size_t i64_length)
{
  spi_transaction_t master_rec = {
    .tx_buffer = NULL,
    .rx_buffer = u8_rx_data,
    .length    = i64_length * 8,
  };
  spi_device_transmit(spi_handle, &master_rec);
}

uint8_t
BSP_spiReadByte (void)
{
  uint8_t u8_rx_data = 0;
  BSP_spiReadFromDevice(&u8_rx_data, 1);
  return u8_rx_data;
}

void
BSP_spiSelectDevice (gpio_num_t e_gpio_pin)
{
  gpio_set_level(e_gpio_pin, 0);
}

void
BSP_spiDeselectDevice (gpio_num_t e_gpio_pin)
{
  gpio_set_level(e_gpio_pin, 1);
}

void
BSP_spiSChangeClock (spi_host_device_t e_spi_host, int i_clock_speed_hz)
{
  spi_device_interface_config_t devcfg
      = { .clock_speed_hz = i_clock_speed_hz, .queue_size = 50 };

  spi_bus_add_device(e_spi_host, &devcfg, &spi_handle);
}