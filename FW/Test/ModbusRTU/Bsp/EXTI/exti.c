/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "esp_attr.h"
#include "esp_intr_alloc.h"

#include "exti.h"

/*****************************************************************************
 *   PUBLIC FUNCTIONS
 *****************************************************************************/
static exti_callback_t exti_user_callback
    = NULL; // Store user callback function

// UART RX Interrupt Handler
void IRAM_ATTR
EXTI_ISR_Handler (void *arg)
{
  if (exti_user_callback)
  {
    exti_user_callback(arg); // Call user-defined function in ISR
  }
}

// Initialize EXTI for a specific GPIO pin
void
EXTI_Init (gpio_num_t pin, gpio_int_type_t intr_type, exti_callback_t callback)
{
  exti_user_callback = callback;

  gpio_config_t io_conf
      = { .intr_type    = intr_type, // Interrupt type (FALLING, RISING, ANY)
          .mode         = GPIO_MODE_INPUT, // Set as input
          .pin_bit_mask = (1ULL << pin),
          .pull_up_en   = GPIO_PULLUP_ENABLE,
          .pull_down_en = GPIO_PULLDOWN_DISABLE };
  gpio_config(&io_conf);
  
  // Attach interrupt to GPIO
  // printf("EXTI initialized on GPIO %d", pin);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(pin, EXTI_ISR_Handler, (void *)pin);
}