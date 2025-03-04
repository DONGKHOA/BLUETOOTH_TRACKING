/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "exti.h"
#include "esp_attr.h"
#include "gpio.h"

/******************************************************************************
 *      PRIVATE DATA
 *****************************************************************************/

static EXIT_CALLBACK_FUNCTION_t EXIT_Callback_Function = NULL;
static uint8_t                  u8_is_initialize       = 0;

/*****************************************************************************
 *   PRIVATE FUNCTIONS
 *****************************************************************************/

static void IRAM_ATTR
BSP_EXTI_Handler (void *argument)
{
  uint32_t u32_io_pin = (uint32_t)argument;

  if (EXIT_Callback_Function != NULL)
  {
    EXIT_Callback_Function(u32_io_pin);
  }
}

/******************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

void
BSP_EXIT_Init (gpio_num_t       e_gpio_pin,
               EXTI_EDGE_t      e_edge,
               gpio_type_pull_t e_gpio_res_pull)
{
  if (u8_is_initialize == 0)
  {
    gpio_install_isr_service(0);
    u8_is_initialize = 1;
  }
  gpio_config_t io_config = {
    .pin_bit_mask = 1ULL << e_gpio_pin,
    .mode         = GPIO_MODE_INPUT,
    .pull_up_en   = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = e_edge,
  };

  gpio_config(&io_config);
  gpio_isr_handler_add(e_gpio_pin, BSP_EXTI_Handler, (void *)e_gpio_pin);
}

void
BSP_EXIT_SetCallBackFunction (EXIT_CALLBACK_FUNCTION_t callbackFunction)
{
  EXIT_Callback_Function = callbackFunction;
}