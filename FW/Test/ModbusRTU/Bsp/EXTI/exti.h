#ifndef EXTI_H
#define EXTI_H

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

  typedef void (*exti_callback_t)(
      void *arg); // Function pointer type for ISR callback

  /*****************************************************************************
   *   PUBLIC FUNCTIONS
   *****************************************************************************/

  void EXTI_Init(gpio_num_t      pin,
                 gpio_int_type_t intr_type,
                 exti_callback_t callback);
  void EXTI_ISR_Handler(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* EXTI_H */