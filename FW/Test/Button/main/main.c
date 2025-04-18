/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "gpio.h"
#include "exti.h"
#include "button.h"

static BUTTON_INF_t button_0;
static TaskHandle_t  exti_handle_task = NULL;

static void
EXIT_Task_Handle ()
{
  while (1)
  {
    DEV_BUTTON_Handle(&button_0);
  }
}

/******************************************************************************
 *     MAIN FUNCTION
 *****************************************************************************/

void
app_main (void)
{
  button_0.e_gpio_pin = GPIO_NUM_33;
  button_0.e_button_state = BUTTON_RELEASING;

  BSP_EXIT_Init(GPIO_NUM_33, GPIO_MODE_INPUT, GPIO_PULL_UP);

  DEV_BUTTON_Init();
  printf("BUTTON_RELEASING\r\n\r\n\r\n");

  xTaskCreate(
      EXIT_Task_Handle, "exti_handle_task", 1024, 0, 15, &exti_handle_task);
}
