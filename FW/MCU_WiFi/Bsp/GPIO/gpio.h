#ifndef GPIO_H
#define GPIO_H

/******************************************************************************
 *   INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************
   *   PUBLIC TYPEDEFS
   *****************************************************************************/

  typedef enum
  {
    GPIO_PULL_UP,
    GPIO_PULL_DOWN,
    GPIO_NO_PULL,
  } gpio_type_pull_t;

  /*****************************************************************************
   *   PUBLIC FUNCTIONS
   *****************************************************************************/

  /**
   * The function `BSP_gpioSetDirection` sets the direction and pull type of a
   * specified GPIO pin.
   *
   * @param e_gpio_pin The `e_gpio_pin` parameter represents the GPIO pin number
   * that you want to configure.
   * @param e_gpio_mode The `e_gpio_mode` parameter specifies the mode
   * (input/output) for the GPIO pin.
   * @param e_gpio_res_pull The `e_gpio_res_pull` parameter specifies the
   * pull-up/pull-down configuration for the GPIO pin.
   */
  void BSP_gpioSetDirection(gpio_num_t       e_gpio_pin,
                            gpio_mode_t      e_gpio_mode,
                            gpio_type_pull_t e_gpio_res_pull);
  /**
   * The function GPIO_SetState sets the state of a GPIO pin to a specified
   * level.
   *
   * @param e_gpio_pin The `e_gpio_pin` parameter represents the GPIO pin number
   * that you want to set the state for.
   * @param u8_state The `state` parameter in the `GPIO_SetState` function is a
   * uint8_t type variable that represents the desired state to set for the
   * specified GPIO pin. It can have a value of either 0 or 1, where 0 typically
   * represents a low state (logic 0) and
   */
  static inline void BSP_gpioSetState (gpio_num_t e_gpio_pin, uint8_t u8_state)
  {
    gpio_set_level(e_gpio_pin, u8_state);
  }

  /**
   * The function `GPIO_GetState` returns the state of a specified GPIO pin.
   *
   * @param e_gpio_pin The `e_gpio_pin` parameter is of type `gpio_num_t`, which
   * is typically used to represent a GPIO pin number in embedded systems
   * programming. It is an enumeration or typedef that maps to the physical GPIO
   * pin numbers on the microcontroller or development board you are working
   * with. When you call `GPIO
   *
   * @return The function `GPIO_GetState` is returning the state of the GPIO pin
   * specified by `e_gpio_pin`. It is using the `gpio_get_level` function to get
   * the level of the GPIO pin and returning it as a `uint8_t` value.
   */
  static inline uint8_t BSP_gpioGetState (gpio_num_t e_gpio_pin)
  {
    return gpio_get_level(e_gpio_pin);
  }

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */