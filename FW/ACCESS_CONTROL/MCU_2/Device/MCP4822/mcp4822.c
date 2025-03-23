/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "esp_log.h"

#include "mcp4822.h"

/******************************************************************************
 *      PRIVATE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *      PUBLIC FUNCTIONS
 *****************************************************************************/

void
DEV_MCP4822_SetValue (spi_device_handle_t spi_handle,
                      mcp4822_channel_t   e_channel,
                      mcp4822_gain_t      e_gain,
                      uint8_t             u8_cs_io,
                      uint16_t            u16_value)
{
  if (u16_value > 4095)
  {
    u16_value = 4095; // Limit 12 bit
  }

  uint16_t command = (e_channel << 15) | (1 << 14) | (e_gain << 13) | (1 << 12)
                     | (u16_value & 0x0FFF);
  uint8_t data[2] = { command >> 8, command & 0xFF };

  ESP_LOGI("MCP4822",
           "Channel: %d, Gain: %d, Value: %d\n",
           e_channel,
           e_gain,
           u16_value);

  BSP_spiDeselectDevice(u8_cs_io);

  BSP_spiWriteMultipleByte(spi_handle, data, 2);

  BSP_spiSelectDevice(u8_cs_io);
}
