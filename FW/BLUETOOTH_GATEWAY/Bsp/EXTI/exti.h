#ifndef EXTI_H
#define EXTI_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************
   *   PUBLIC TYPEDEFS
   *****************************************************************************/

  typedef void (*EXIT_CALLBACK_FUNCTION_t)(gpio_num_t gpio_pin);

  typedef enum
  {
    EXTI_EDGE_RISING  = 1,
    EXTI_EDGE_FALLING = 2,
    EXTI_EDGE_ALL     = 3,
  } EXTI_EDGE_t;

  /*****************************************************************************
   *   PUBLIC FUNCTIONS
   *****************************************************************************/

  void BSP_EXIT_Init(gpio_num_t       e_gpio_pin,
                     EXTI_EDGE_t      e_edge,
                     gpio_type_pull_t e_gpio_res_pull);
  void BSP_EXIT_SetCallBackFunction(EXIT_CALLBACK_FUNCTION_t callbackFunction);

#ifdef __cplusplus
}
#endif

#endif