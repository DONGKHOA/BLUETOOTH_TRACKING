/******************************************************************************
 *      INCLUDES
 ******************************************************************************/

#include <stdio.h>

#include "app_data.h"
#include "app_user_button.h"

#include "gpio.h"
#include "exti.h"

/******************************************************************************
 *    PRIVATE DEFINES
 *****************************************************************************/

#define TAG "APP_USER_BUTTON"

#define GET_TICK (xTaskGetTickCount() * portTICK_PERIOD_MS)

#define PRESSING_TIMEOUT   3000 // ms
#define DEBOUNCING_TIMEOUT 100  // ms

/******************************************************************************
 *    PRIVATE TYPEDEFS
 *****************************************************************************/

typedef struct
{
  QueueHandle_t       s_button_queue_data;
  TimerHandle_t       s_button_time_pressing;
  TimerHandle_t       s_button_time_debouncing;
  EventGroupHandle_t *p_button_event;
  uint8_t             u8_is_debounce_pressing;
  uint8_t             u8_is_debounce_releasing;
  uint8_t             u8_is_long_pressing;
} user_button_t;

/*****************************************************************************
 *   PRIVATE PROTOTYPE FUNCTIONS
 *****************************************************************************/

// static void APP_USER_BUTTON_Task(void *arg);
static void APP_USER_BUTTON_Func_cb(gpio_num_t e_pin);
static void APP_USER_BUTTON_Timer_Pressing();
static void APP_USER_BUTTON_Timer_Debouncing();

/******************************************************************************
 *    PRIVATE DATA
 *****************************************************************************/

static user_button_t s_user_button;

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_USER_BUTTON_Init (void)
{
  s_user_button.p_button_event      = &s_data_system.s_configuration_event;
  s_user_button.s_button_queue_data = xQueueCreate(1, sizeof(uint8_t));
  s_user_button.s_button_time_pressing
      = xTimerCreate("button_time_pressing",
                     PRESSING_TIMEOUT / portTICK_PERIOD_MS,
                     pdFALSE,
                     (void *)0,
                     APP_USER_BUTTON_Timer_Pressing);
  s_user_button.s_button_time_debouncing
      = xTimerCreate("button_time_debouncing",
                     DEBOUNCING_TIMEOUT / portTICK_PERIOD_MS,
                     pdFALSE,
                     (void *)0,
                     APP_USER_BUTTON_Timer_Debouncing);

  xTimerStop(s_user_button.s_button_time_pressing, 0);
  xTimerStop(s_user_button.s_button_time_debouncing, 0);

  s_user_button.u8_is_debounce_pressing  = 0;
  s_user_button.u8_is_debounce_releasing = 0;
  s_user_button.u8_is_long_pressing      = 0;

  BSP_EXIT_SetCallBackFunction(APP_USER_BUTTON_Func_cb);
}

/*****************************************************************************
 *   PRIVATE FUNCTIONS
 *****************************************************************************/

static void
APP_USER_BUTTON_Func_cb (gpio_num_t e_pin)
{
  if (e_pin != BUTTON_USER_PIN)
  {
    return;
  }

  if (xTimerIsTimerActive(s_user_button.s_button_time_debouncing) == pdTRUE)
  {
    return;
  }

  xTimerStart(s_user_button.s_button_time_debouncing, 0);
}

static void
APP_USER_BUTTON_Timer_Debouncing ()
{
  uint8_t u8_state = BSP_gpioGetState(BUTTON_USER_PIN);

  if (u8_state == 0)
  {
    if (s_user_button.u8_is_debounce_pressing == 0)
    {
      s_user_button.u8_is_debounce_pressing = 1;
      xTimerStart(s_user_button.s_button_time_pressing, 0);
    }
  }
  else
  {
    if (s_user_button.u8_is_debounce_pressing == 1)
    {
      s_user_button.u8_is_debounce_pressing = 0;
      xTimerStop(s_user_button.s_button_time_pressing, 0);
      xTimerStop(s_user_button.s_button_time_debouncing, 0);

      if (s_user_button.u8_is_long_pressing)
      {
        s_user_button.u8_is_long_pressing = 0;
        printf("Button Long Pressed\r\n");
      }
      else
      {
        printf("Button Short Pressed\r\n");
        xEventGroupSetBits(*s_user_button.p_button_event,
                           BUTTON_Short_Pressing_BIT);
      }
    }
  }
}

static void
APP_USER_BUTTON_Timer_Pressing ()
{
  xEventGroupSetBits(*s_user_button.p_button_event, BUTTON_Long_Pressing_BIT);
  s_user_button.u8_is_long_pressing = 1;
}