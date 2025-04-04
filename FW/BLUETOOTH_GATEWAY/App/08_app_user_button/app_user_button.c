/******************************************************************************
 *      INCLUDES
 ******************************************************************************/

#include <stdio.h>

#include "app_data.h"
#include "app_user_button.h"

#include "gpio.h"
#include "exti.h"

/*****************************************************************************
 *   PRIVATE PROTOTYPE FUNCTIONS
 *****************************************************************************/

static void APP_USER_BUTTON_Func_cb(uint32_t u32_pin);
static void APP_USER_BUTTON_Task(void *arg);

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

void
APP_USER_BUTTON_CreateTask (void)
{
}

void
APP_USER_BUTTON_Init (void)
{
  BSP_EXIT_SetCallBackFunction(APP_USER_BUTTON_Func_cb);
}

/*****************************************************************************
 *   PRIVATE FUNCTIONS
 *****************************************************************************/

static void
APP_USER_BUTTON_Func_cb (uint32_t u32_pin)
{
}