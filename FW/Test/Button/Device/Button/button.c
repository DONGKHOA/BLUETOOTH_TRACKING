/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "button.h"
#include "exti.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

/******************************************************************************
 *   PRIVATE DEFINE
 *****************************************************************************/

#define TIME_LONG_PRESSING 3000 // 30x100ms
#define TIME_DEBOUNCE      100  // 1x100ms

/******************************************************************************
 *   PRIVATE TYPEDEFS
 *****************************************************************************/

typedef enum
{
  BUTTON_STATE_MACHINE_RELEASING = 0,
  BUTTON_STATE_MACHINE_DEBOUNCE_PRESSING,
  BUTTON_STATE_MACHINE_WAIT_PRESSING,
  BUTTON_STATE_MACHINE_LONG_PRESSING,
  BUTTON_STATE_MACHINE_DEBOUNCE_RELEASING
} BUTTON_StateMachine_t;

typedef struct
{
  QueueHandle_t         s_exti_event_queue;
  TimerHandle_t         s_time_button;
  gpio_num_t            e_gpio_pin;
  BUTTON_StateMachine_t e_button_state_machine;
  uint16_t               u8_time_counter;
} BUTTON_Handle_t;

/******************************************************************************
 *      PRIVATE DATA
 *****************************************************************************/

static BUTTON_Handle_t s_hbutton;

/*****************************************************************************
 *   PRIVATE PROTOTYPE FUNCTIONS
 *****************************************************************************/

static void DEV_BUTTON_Func_cb(uint32_t u32_pin);
static void DEV_BUTTON_TimerCallback(TimerHandle_t s_Timer);

/******************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

void
DEV_BUTTON_Init (void)
{
  s_hbutton.s_exti_event_queue = xQueueCreate(32, sizeof(uint32_t));
  s_hbutton.s_time_button      = xTimerCreate("time_button",
                                         1 / portTICK_PERIOD_MS,
                                         pdTRUE,
                                         (void *)0,
                                         DEV_BUTTON_TimerCallback);

  s_hbutton.e_button_state_machine = BUTTON_STATE_MACHINE_RELEASING;
  s_hbutton.u8_time_counter        = 0;

  BSP_EXIT_SetCallBackFunction(DEV_BUTTON_Func_cb);
}

void
DEV_BUTTON_Handle (BUTTON_INF_t *p_button)
{
  if (xQueueReceive(
          s_hbutton.s_exti_event_queue, &s_hbutton.e_gpio_pin, portMAX_DELAY))
  {
    if (p_button->e_gpio_pin == s_hbutton.e_gpio_pin)
    {
      // printf("state: %d\r\n", s_hbutton.e_button_state_machine);
      switch (s_hbutton.e_button_state_machine)
      {
        case BUTTON_STATE_MACHINE_RELEASING:

          s_hbutton.u8_time_counter = TIME_DEBOUNCE;
          s_hbutton.e_button_state_machine
              = BUTTON_STATE_MACHINE_DEBOUNCE_PRESSING;

          xTimerReset(s_hbutton.s_time_button, 0);

          break;

        case BUTTON_STATE_MACHINE_DEBOUNCE_PRESSING:

          if (s_hbutton.u8_time_counter == 0)
          {
            printf("BUTTON_SHORT_PRESSING\r\n\r\n\r\n");
            s_hbutton.u8_time_counter = TIME_LONG_PRESSING;
            xTimerReset(s_hbutton.s_time_button, 0);

            p_button->e_button_state = BUTTON_SHORT_PRESSING;
            s_hbutton.e_button_state_machine
                = BUTTON_STATE_MACHINE_WAIT_PRESSING;
          }

          break;

        case BUTTON_STATE_MACHINE_WAIT_PRESSING:

          if (s_hbutton.u8_time_counter == 0)
          {
            printf("BUTTON_LONG_PRESSING\r\n\r\n\r\n");
            xTimerStop(s_hbutton.s_time_button, 0);

            p_button->e_button_state = BUTTON_LONG_PRESSING;
            s_hbutton.e_button_state_machine
                = BUTTON_STATE_MACHINE_LONG_PRESSING;
          }
          else
          {
            s_hbutton.u8_time_counter = TIME_DEBOUNCE;
            xTimerReset(s_hbutton.s_time_button, 0);

            s_hbutton.e_button_state_machine
                = BUTTON_STATE_MACHINE_DEBOUNCE_RELEASING;
          }

          break;

        case BUTTON_STATE_MACHINE_LONG_PRESSING:

          s_hbutton.u8_time_counter = TIME_DEBOUNCE;
          xTimerReset(s_hbutton.s_time_button, 0);

          s_hbutton.e_button_state_machine
              = BUTTON_STATE_MACHINE_DEBOUNCE_RELEASING;

          break;

        case BUTTON_STATE_MACHINE_DEBOUNCE_RELEASING:

          if (s_hbutton.u8_time_counter == 0)
          {
            printf("BUTTON_RELEASING\r\n\r\n\r\n");
            xTimerStop(s_hbutton.s_time_button, 0);

            p_button->e_button_state         = BUTTON_RELEASING;
            s_hbutton.e_button_state_machine = BUTTON_STATE_MACHINE_RELEASING;
          }

          break;

        default:
          break;
      }
    }
  }
}

/*****************************************************************************
 *   PRIVATE FUNCTIONS
 *****************************************************************************/

static void
DEV_BUTTON_TimerCallback (TimerHandle_t s_Timer)
{
  if (s_hbutton.u8_time_counter == 0)
  {
    xQueueSendFromISR(
        s_hbutton.s_exti_event_queue, &s_hbutton.e_gpio_pin, NULL);
  }
  else
  {
    s_hbutton.u8_time_counter--;
  }
}

static void
DEV_BUTTON_Func_cb (uint32_t u32_pin)
{
  xQueueSendFromISR(s_hbutton.s_exti_event_queue, &u32_pin, NULL);
}