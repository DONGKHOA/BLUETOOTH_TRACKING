/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "gpio.h"

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

void
BSP_gpioInputInit (gpio_num_t e_gpio_pin, gpio_type_pull_t e_gpio_res_pull)
{
  if (e_gpio_res_pull == GPIO_NO_PULL)
  {
    gpio_config_t io_conf = {
      .pin_bit_mask = (1 << e_gpio_pin),
      .mode         = GPIO_MODE_INPUT,
      .pull_up_en   = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
  }
  else
  {
    gpio_config_t io_conf = {
      .pin_bit_mask = (1 << e_gpio_pin),
      .mode         = GPIO_MODE_INPUT,
      .pull_up_en   = !e_gpio_res_pull,
      .pull_down_en = e_gpio_res_pull,
      .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
  }
}

void
BSP_gpioOutputInit (gpio_num_t e_gpio_pin)
{
  gpio_config_t io_config = {
    .mode         = GPIO_MODE_OUTPUT,
    .pin_bit_mask = (1 << e_gpio_pin),
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_config);
}

void
BSP_gpioSetState (gpio_num_t e_gpio_pin, uint8_t u8_state)
{
  gpio_set_level(e_gpio_pin, u8_state);
}

uint8_t
BSP_gpioGetState (gpio_num_t e_gpio_pin)
{
  return gpio_get_level(e_gpio_pin);
}

void
BSP_gpioSetDirection (gpio_num_t e_gpio_pin, gpio_mode_t e_gpio_mode)
{
  gpio_set_direction(e_gpio_pin, e_gpio_mode);
}