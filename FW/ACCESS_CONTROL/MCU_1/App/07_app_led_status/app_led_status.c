/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_led_status.h"
#include "app_data.h"

#include "gpio.h"

#define TAG             "APP_LED_STATUS"
#define TIME_LED_STATUS 100 / portTICK_PERIOD_MS

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static uint32_t        u32_time_count_1 = 0;
static uint32_t        u32_time_count_2 = 0;
static state_system_t *p_state_system;
static TimerHandle_t   led_status_timer;

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

  *p_state_system = STATE_IDLE;

  led_status_timer = xTimerCreate(
      "Led Status Timer", TIME_LED_STATUS, pdTRUE, NULL, APP_Timer_Callback);

  xTimerStart(led_status_timer, 0);
}

static void
APP_Timer_Callback (TimerHandle_t xTimer)
{
  switch (*p_state_system)
  {
    case STATE_IDLE:

      if (u32_time_count_1 >= 15)
      {
        TOGGLE_LED();
        u32_time_count_1 = 0;
      }

      break;

    case STATE_ATTENDANCE:

      if (u32_time_count_1 >= 5)
      {
        TOGGLE_LED();
        u32_time_count_1 = 0;
      }

      break;

    case STATE_ATTENDANCE_SUCCESS:

      if (u32_time_count_2 >= 30)
      {
        *p_state_system = STATE_IDLE;
        LED_OFF();
        u32_time_count_1 = 0;
        u32_time_count_2 = 0;
      }
      else
      {
        LED_ON();
        u32_time_count_2++;
      }

      break;

    case STATE_ATTENDANCE_FAIL:

      if (u32_time_count_2 >= 30)
      {
        *p_state_system = STATE_IDLE;
        LED_OFF();
        u32_time_count_1 = 0;
        u32_time_count_2 = 0;
      }

      else if (u32_time_count_1 >= 1)
      {
        TOGGLE_LED();
        u32_time_count_1 = 0;
        u32_time_count_2++;
      }

      break;

    case STATE_ENROLL_SUCCESS:

      if (u32_time_count_2 >= 30)
      {
        *p_state_system = STATE_IDLE;
        LED_OFF();
        u32_time_count_1 = 0;
        u32_time_count_2 = 0;
      }
      else
      {
        LED_ON();
        u32_time_count_2++;
      }

      break;

    case STATE_ENROLL_FAIL:

      if (u32_time_count_2 >= 30)
      {
        *p_state_system = STATE_IDLE;
        LED_OFF();
        u32_time_count_1 = 0;
        u32_time_count_2 = 0;
      }

      else if (u32_time_count_1 >= 1)
      {
        TOGGLE_LED();
        u32_time_count_1 = 0;
        u32_time_count_2++;
      }

      break;

    default:
      break;
  }

  u32_time_count_1++;
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