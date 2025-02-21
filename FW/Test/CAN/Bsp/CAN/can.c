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

static void (*IsrFunctionPointer)(void *)
    = NULL; // Function pointer for Dev ISR

/*****************************************************************************
 *      PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void IRAM_ATTR BSP_canIsrHandler(void *arg);

volatile bool isr_triggered = false; // Debug flag
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
BSP_canConfigInterrupt (void)
{
  g_config.intr_flags = ESP_INTR_FLAG_IRAM;

  esp_err_t err = twai_reconfigure_alerts(TWAI_ALERT_RX_DATA, NULL);
  if (err == ESP_OK)
  {
    ESP_LOGI("DEBUG", "TWAI alerts enabled!");
  }
  else
  {
    ESP_LOGE("ERROR", "Failed to enable TWAI alerts: %d", err);
  }
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

// Function to register the high-layer ISR
void
BSP_RegisterIsr (void (*isr_func)(void *))
{
  IsrFunctionPointer = isr_func;
}

/*****************************************************************************
 *      PUBLIC FUNCTION
 *****************************************************************************/
static void IRAM_ATTR
BSP_canIsrHandler (void *arg)
{
  isr_triggered = true;
  printf("Trigger\n");
  uint32_t alerts;

  twai_read_alerts(&alerts, pdMS_TO_TICKS(10));

  // If data is received, call the high-layer function (if registered)
  if ((alerts & TWAI_ALERT_RX_DATA))
  {
    printf("CAN RX Interrupt detected!\n");
    if (IsrFunctionPointer != NULL)
    {
      printf("Calling high-layer ISR...\n");
      IsrFunctionPointer(arg); // Call the registered ISR
    }
    else
    {
      printf("ERROR: IsrFunctionPointer is NULL!\n");
    }
  }
}