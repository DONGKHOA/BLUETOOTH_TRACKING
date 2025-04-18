#ifndef BUTTON_H
#define BUTTON_H

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef enum
  {
    BUTTON_RELEASING = 0,
    BUTTON_SHORT_PRESSING,
    BUTTON_LONG_PRESSING
  } BUTTON_State_t;

  typedef struct
  {
    gpio_num_t     e_gpio_pin;
    BUTTON_State_t e_button_state;
  } BUTTON_INF_t;

  /****************************************************************************
   *   PUBLIC FUNCTIONS
   ***************************************************************************/

  void DEV_BUTTON_Init(void);
  void DEV_BUTTON_Handle(BUTTON_INF_t *p_button);

#ifdef __cplusplus
}
#endif

#endif