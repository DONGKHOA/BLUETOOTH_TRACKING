#include <stdio.h>
#include <stdint.h>

static void
TestMain_Fingerprint_Register_Task (void *pvParameters)
{
  uint16_t u16_stored_fingerprints = 0;
  uint16_t u16_user_id             = 0;
  uint8_t  confirmation_code       = 0;
  uint8_t  page_id[2]              = { 0 };

  while (1)
  {
    // Get the number of saved fingerprint templates
    DEV_AS608_TempleteNum(
        UART_PORT_NUM_2, default_address, &u16_stored_fingerprints);
    u16_user_id = u16_stored_fingerprints
                  + 1; // add new fingerprint = store templates + 1 (new user)
    page_id[0] = ((u16_user_id >> 8) & 0xFF);
    page_id[1] = (u16_user_id & 0xFF);
    printf("User ID: %d\n", u16_user_id);

    // Get the first fingerprint image
    confirmation_code = DEV_AS608_GenImg(UART_PORT_NUM_2, default_address);

    if (confirmation_code != 0x00)
    {
      continue;
    }
      confirmation_code
          = DEV_AS608_Img2Tz(UART_PORT_NUM_2, default_address, buffer1);
          if (confirmation_code != 0x00)
          {  
          }  
          printf("Remove your finger\n");
          vTaskDelay(pdMS_TO_TICKS(2000));
    
          // Get the second fingerprint image
          confirmation_code = DEV_AS608_GenImg(UART_PORT_NUM_2, default_address);
      if (confirmation_code == 0x00)
      {
        if (confirmation_code == 0x00)
        {
          if (confirmation_code == 0x00)
          {
            confirmation_code
                = DEV_AS608_Img2Tz(UART_PORT_NUM_2, default_address, buffer2);
            if (confirmation_code == 0x00)
            {
              // Merge fingerprint features confirmation_code
              confirmation_code
                  = DEV_AS608_RegModel(UART_PORT_NUM_2, default_address);
              if (confirmation_code == 0)
              {
                // Check that the fingerprint template exists in flash or not
                confirmation_code = DEV_AS608_Search(UART_PORT_NUM_2,
                                                     default_address,
                                                     buffer1,
                                                     start_page,
                                                     page_number);
                if (confirmation_code != 0)
                {
                  confirmation_code = DEV_AS608_Store(
                      UART_PORT_NUM_2, default_address, buffer1, page_id);
                  if (confirmation_code == 0)
                  {
                    printf("Enroll success! Stored template with ID: %d\n",
                           u16_user_id);
                  }
                  else
                  {
                    printf("Error: Cannot store template | %d\n",
                           confirmation_code);
                  }
                }
              }
            }
          }
        }
      }
    }
  }