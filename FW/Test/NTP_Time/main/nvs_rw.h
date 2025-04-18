#ifndef NVS_H_
#define NVS_H_

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include "esp_err.h"

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

esp_err_t NVS_Init(void);
esp_err_t NVS_WriteString(const char* name, const char* key, 
                            const char* stringVal);

esp_err_t NVS_ReadString(const char* name, const char* key, 
                            char* savedData, uint8_t len);

#endif /* NVS_H_ */