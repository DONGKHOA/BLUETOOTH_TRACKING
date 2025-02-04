#ifndef GPIO_H
#define GPIO_H

/******************************************************************************
 *   INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/gpio.h"

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
 * The function `GPIO_Input_Init` initializes a GPIO pin as an input with
optional pull-up or pull-down
 * resistor configuration.
 *
 * @param e_gpio_pin The `e_gpio_pin` parameter is of type `gpio_num_t`, which
is typically an enumeration
 * representing the GPIO pin number on a microcontroller. It is used to specify
the GPIO pin for which
 * the input configuration needs to be initialized.
 * @param e_gpio_res_pull The `e_gpio_res_pull` parameter is used to specify the
pull configuration for the
 * GPIO pin. It can have two values:
 */
void BSP_gpioInputInit(gpio_num_t e_gpio_pin, gpio_type_pull_t e_gpio_res_pull);

/**
 * The function `GPIO_Output_Init` initializes a GPIO pin as an output with
 * specific configuration settings.
 *
 * @param e_gpio_pin The `e_gpio_pin` parameter is the GPIO pin number that you
 * want to initialize as an output pin.
 */
void BSP_gpioOutputInit(gpio_num_t e_gpio_pin);

/**
 * The function GPIO_SetState sets the state of a GPIO pin to a specified level.
 *
 * @param e_gpio_pin The `e_gpio_pin` parameter represents the GPIO pin number
 * that you want to set the state for.
 * @param u8_state The `state` parameter in the `GPIO_SetState` function is a
 * uint8_t type variable that represents the desired state to set for the
 * specified GPIO pin. It can have a value of either 0 or 1, where 0 typically
 * represents a low state (logic 0) and
 */
void BSP_gpioSetState(gpio_num_t e_gpio_pin, uint8_t u8_state);

/**
 * The function `GPIO_GetState` returns the state of a specified GPIO pin.
 *
 * @param e_gpio_pin The `e_gpio_pin` parameter is of type `gpio_num_t`, which
 * is typically used to represent a GPIO pin number in embedded systems
 * programming. It is an enumeration or typedef that maps to the physical GPIO
 * pin numbers on the microcontroller or development board you are working with.
 * When you call `GPIO
 *
 * @return The function `GPIO_GetState` is returning the state of the GPIO pin
 * specified by `e_gpio_pin`. It is using the `gpio_get_level` function to get
 * the level of the GPIO pin and returning it as a `uint8_t` value.
 */
uint8_t BSP_gpioGetState(gpio_num_t e_gpio_pin);

/**
 * @brief Set the direction of a specified GPIO pin.
 *
 * This function configures the direction of the specified GPIO pin.
 *
 * @param e_gpio_pin The GPIO pin number to configure.
 * @param e_gpio_mode The mode to set for the GPIO pin (input, output, etc.).
 */
void BSP_gpioSetDirection(gpio_num_t e_gpio_pin, gpio_mode_t e_gpio_mode);

#endif /* GPIO_H */