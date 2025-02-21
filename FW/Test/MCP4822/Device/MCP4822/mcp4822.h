#ifndef MCP4822_H_
#define MCP4822_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "gpio.h"
#include "spi.h"

/*****************************************************************************
 *      PUBLIC TYPEDEFS
 *****************************************************************************/

typedef enum
{
  MCP4822_DAC_A,
  MCP4822_DAC_B
} mcp4822_channel_t;

typedef enum
{
  MCP4822_OUTPUT_GAIN_x2,
  MCP4822_OUTPUT_GAIN_x1
} mcp4822_gain_t;

/*****************************************************************************
 *      PUBLIC FUNCTIONS
 *****************************************************************************/

/**
 * @brief Sets the value for the specified channel of the MCP4822 DAC.
 *
 * @param u8_channel The channel to set the value for (0 or 1).
 * @param u8_gain The gain setting (0 for 2x, 1 for 1x).
 * @param u16_value The value to set (0 to 4095).
 */
void DEV_MCP4822_SetValue(mcp4822_channel_t e_channel,
                          mcp4822_gain_t    e_gain,
                          uint8_t           u8_cs_io,
                          uint16_t          u16_value);

/**
 * @brief Outputs the current value to the MCP4822 DAC.
 */
void DEV_MPC4822_Output(uint8_t u8_ldac_io);

#endif /* MCP4822_H_ */