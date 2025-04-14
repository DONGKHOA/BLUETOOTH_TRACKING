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

static void APP_STATUS_LED_Task(void *arg);
static void APP_Timer_Callback(TimerHandle_t xTimer);

static void TOGGLE_LED(void);
static void LED_ON(void);
static void LED_OFF(void);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_STATUS_LED_CreateTask (void)
{
  xTaskCreate(APP_STATUS_LED_Task, "Led status task", 1024 * 2, NULL, 4, NULL);
}

void
APP_STATUS_LED_Init (void)
{
  p_state_system = &s_data_system.s_state_system;

  TimerHandle_t led_status_timer
      = xTimerCreate("Led Status Timer",
                     pdMS_TO_TICKS(100), // Period 100ms
                     pdTRUE,             // Auto reload
                     NULL,
                     APP_Timer_Callback // Callback function
      );

  xTimerStart(led_status_timer, 0);
}

static void
APP_Timer_Callback (TimerHandle_t xTimer)
{
  u32_count += 100; // Increase count every 100ms
}

/******************************************************************************
 *  PRIVATE FUNCTION
 *****************************************************************************/

static void
APP_STATUS_LED_Task (void *arg)
{
  uint32_t attendance_success = 0;
  while (1)
  {
    if (p_state_system != NULL)
    {
      switch (*p_state_system)
      {
        case STATE_IDLE:
          if (u32_count % 1000)
          {
            TOGGLE_LED();
            attendance_success = 0;
          }
          break;

        case STATE_ATTENDANCE:
          LED_ON();
          break;

        case STATE_ATTENDANCE_SUCCESS:
          if (u32_count % 5000)
          {
            TOGGLE_LED();
            attendance_success = 1;
          }

          if (attendance_success == 1)
          {
            LED_OFF();
            attendance_success = 0;
          }
          
          break;

        case STATE_ATTENDANCE_FAIL:

          break;

        case STATE_ENROLL_SUCCESS:

          break;

        case STATE_ENROLL_FAIL:

          break;

        default:
          break;
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

static void
TOGGLE_LED (void)
{
  static bool led_status = false;
  led_status             = !led_status;
  BSP_gpioSetState(LED_STATUS_PIN, led_status);
}

static void
LED_ON (void)
{
  BSP_gpioSetState(LED_STATUS_PIN, 1);
}

static void
LED_OFF (void)
{
  BSP_gpioSetState(LED_STATUS_PIN, 0);
}