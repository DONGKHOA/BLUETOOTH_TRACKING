/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "can.h"
#include "sdkconfig.h"

/*****************************************************************************
 *      PRIVATE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

void
BSP_canDriverInit (twai_mode_t           e_can_mode,
                   gpio_num_t            e_tx_pin,
                   gpio_num_t            e_rx_pin,
                   uint32_t              u32_tx_queue_len,
                   uint32_t              u32_rx_queue_len,
                   int                   i_intr_flag,
                   can_config_bit_rate_t e_bitrate)
{
  twai_general_config_t g_config = { .mode           = e_can_mode,
                                     .tx_io          = e_tx_pin,
                                     .rx_io          = e_rx_pin,
                                     .clkout_io      = TWAI_IO_UNUSED,
                                     .bus_off_io     = TWAI_IO_UNUSED,
                                     .tx_queue_len   = u32_tx_queue_len,
                                     .rx_queue_len   = u32_rx_queue_len,
                                     .alerts_enabled = TWAI_ALERT_NONE,
                                     .clkout_divider = 0,
                                     .intr_flags     = i_intr_flag };

  twai_timing_config_t t_config = { 0 };

  switch (e_bitrate)
  {
    case CAN_25KBITS:
      t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_25KBITS();
      break;

    case CAN_50KBITS:
      t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_50KBITS();
      break;

    case CAN_100KBITS:
      t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_100KBITS();
      break;

    case CAN_125KBITS:
      t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_125KBITS();
      break;

    case CAN_250KBITS:
      t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_250KBITS();
      break;

    case CAN_500KBITS:
      t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_500KBITS();
      break;

    case CAN_800KBITS:
      t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_800KBITS();
      break;

    case CAN_1MBITS:
      t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_1MBITS();
      break;

    default:
      break;
  }

  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  twai_driver_install(&g_config, &t_config, &f_config);
}

esp_err_t
BSP_canStart (void)
{
  esp_err_t response = twai_start();
  return response;
}

esp_err_t
BSP_canStop (void)
{
  esp_err_t response = twai_stop();
  return response;
}

esp_err_t
BSP_canTransmit (twai_message_t *p_message, TickType_t u32_timeout)
{
  esp_err_t response = twai_transmit(p_message, u32_timeout);
  return response;
}

esp_err_t
BSP_canReceive (twai_message_t *p_message, TickType_t u32_timeout)
{
  esp_err_t response = twai_receive(p_message, u32_timeout);
  return response;
}