/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "gpio.h"

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/

void
BSP_gpioSetDirection (gpio_num_t       e_gpio_pin,
                      gpio_mode_t      e_gpio_mode,
                      gpio_type_pull_t e_gpio_res_pull)
{
  gpio_set_direction(e_gpio_pin, e_gpio_mode);

  if (e_gpio_mode == GPIO_MODE_OUTPUT)
  {
    if (e_gpio_res_pull & GPIO_PULL_UP)
    {
      gpio_pullup_en(e_gpio_mode);
    }
    else
    {
      gpio_pullup_dis(e_gpio_mode);
    }

    if (e_gpio_res_pull & GPIO_PULL_DOWN)
    {
      gpio_pulldown_en(e_gpio_mode);
    }
    else
    {
      gpio_pulldown_dis(e_gpio_mode);
    }

    if (e_gpio_res_pull & GPIO_NO_PULL)
    {
      gpio_pullup_dis(e_gpio_mode);
      gpio_pulldown_dis(e_gpio_mode);
    }
  }
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