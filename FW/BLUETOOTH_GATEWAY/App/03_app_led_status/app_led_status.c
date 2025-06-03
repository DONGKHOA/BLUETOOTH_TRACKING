/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_led_status.h"
#include "app_data.h"

#include "gpio.h"

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static uint32_t        u32_count = 0;
static state_system_t *p_state_system;

/******************************************************************************
 *  PRIVATE PROTOTYPE FUNCTION
 *****************************************************************************/

static void APP_Timer_Callback(TimerHandle_t xTimer);

static void        TOGGLE_LED(void);
static inline void LED_ON(void);
static inline void LED_OFF(void);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_STATUS_LED_Init (void)
{
  p_state_system = &s_data_system.s_state_system;

  TimerHandle_t led_status_timer = xTimerCreate(
      "Led Status Timer", pdMS_TO_TICKS(100), pdTRUE, NULL, APP_Timer_Callback);

  xTimerStart(led_status_timer, 0);
}

static void
APP_Timer_Callback (TimerHandle_t xTimer)
{
  u32_count += 100; // Increase count every 100ms
  switch (*p_state_system)
  {
    case STATE_WIFI_CONNECTED:
      if (u32_count >= 1000)
      {
        u32_count = 0; // Reset count
        TOGGLE_LED();
      }
      break;

    case STATE_WIFI_DISCONNECTED:
      if (u32_count >= 200)
      {
        u32_count = 0; // Reset count
        TOGGLE_LED();
      }
      break;

    case STATE_BLUETOOTH_CONFIG:
      u32_count = 0; // Reset count
      LED_ON();
      break;

    default:
      break;
  }
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
TOGGLE_LED (void)
{
  static bool led_status = false;
  led_status             = !led_status;
  BSP_gpioSetState(LED_STATUS_PIN, led_status);
}

static inline void
LED_ON (void)
{
  BSP_gpioSetState(LED_STATUS_PIN, 1);
}

static inline void
LED_OFF (void)
{
  BSP_gpioSetState(LED_STATUS_PIN, 0);
}