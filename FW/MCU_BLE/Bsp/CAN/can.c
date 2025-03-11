/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "can.h"
#include "sdkconfig.h"

/*****************************************************************************
 *      PRIVATE VARIABLES
 *****************************************************************************/

static twai_general_config_t g_config = { 0 };
static twai_timing_config_t  t_config = { 0 };
static twai_filter_config_t  f_config = { 0 };

/*****************************************************************************
 *      PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

/*****************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/

esp_err_t
BSP_canDriverInit (void)
{
  return twai_driver_install(&g_config, &t_config, &f_config);
}

void
BSP_canConfigDefault (void)
{
  g_config.clkout_io      = TWAI_IO_UNUSED;
  g_config.bus_off_io     = TWAI_IO_UNUSED;
  g_config.alerts_enabled = TWAI_ALERT_NONE;
  g_config.clkout_divider = 0;

  f_config = (twai_filter_config_t)TWAI_FILTER_CONFIG_ACCEPT_ALL();
}

void
BSP_canConfigMode (twai_mode_t e_can_mode)
{
  g_config.mode = e_can_mode;
}

void
BSP_canConfigIO (gpio_num_t e_tx_pin, gpio_num_t e_rx_pin)
{
  g_config.tx_io = e_tx_pin;
  g_config.rx_io = e_rx_pin;
}

void
BSP_canConfigQueue (uint32_t u32_tx_queue_len, uint32_t u32_rx_queue_len)
{
  g_config.tx_queue_len = u32_tx_queue_len;
  g_config.rx_queue_len = u32_rx_queue_len;
}

void
BSP_canConfigIntr (int i_intr_flag)
{
  g_config.intr_flags = i_intr_flag;
}

void
BSP_canConfigBitRate (can_bit_rate_t e_bitrate)
{
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
}







