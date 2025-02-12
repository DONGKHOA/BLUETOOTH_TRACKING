// /******************************************************************************
//  *      INCLUDES
//  *****************************************************************************/

// #include "esp_log.h"

// #include "mcp4822.h"

// /******************************************************************************
//  *      PRIVATE DEFINES
//  *****************************************************************************/

// #define HSPI_HOST SPI3_HOST
// #define MISO_PIN  -1
// #define MOSI_PIN  GPIO_NUM_19
// #define SCLK_PIN  GPIO_NUM_20
// #define CS_PIN    GPIO_NUM_21
// #define LDAC_PIN  GPIO_NUM_47

// #define CLOCK_SPEED_HZ  1000000
// #define MAX_TRANSFER_SZ 4096
// #define DMA_CHANNEL     3
// #define SPI_MODE        0

// /*****************************************************************************
//  *      PUBLIC FUNCTIONS
//  *****************************************************************************/
// void
// DEV_MCP4822_Init (void)
// {
//   BSP_spiDriverInit(HSPI_HOST,
//                     MISO_PIN,
//                     MOSI_PIN,
//                     SCLK_PIN,
//                     MAX_TRANSFER_SZ,
//                     CS_PIN,
//                     DMA_CHANNEL,
//                     CLOCK_SPEED_HZ,
//                     SPI_MODE);

//   BSP_gpioOutputInit(LDAC_PIN);
//   BSP_gpioSetState(LDAC_PIN, 0);
// }

// void
// DEV_MCP4822_SetValue (uint8_t u8_channel, uint8_t u8_gain, uint16_t u16_value)
// {
//   if (u16_value > 4095)
//   {
//     u16_value = 4095; // Limit 12 bit
//   }

//   uint16_t command = (u8_channel << 15) | (1 << 14) | (u8_gain << 13)
//                      | (1 << 12) | (u16_value & 0x0FFF);
//   uint8_t data[2] = { command >> 8, command & 0xFF };

//   ESP_LOGI("MCP4822",
//            "Channel: %d, Gain: %d, Value: %d\n",
//            u8_channel,
//            u8_gain,
//            u16_value);
//   // ESP_LOGI("MCP4822", "SPI Command Sent: 0x%04X\n", command);

//   BSP_spiDeselectDevice(CS_PIN);

//   BSP_spiWriteToDevice(data, sizeof(data));

//   BSP_spiSelectDevice(CS_PIN);
// }

// void
// DEV_MPC4822_Output (void)
// {
//   BSP_gpioSetState(LDAC_PIN, 0);
//   BSP_gpioSetState(LDAC_PIN, 1);
// }
