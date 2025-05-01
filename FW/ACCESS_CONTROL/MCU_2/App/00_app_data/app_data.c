/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "app_data.h"

/******************************************************************************
 *   PUBLIC DATA
 *****************************************************************************/

DATA_System_t s_data_system;

bool b_valid_sdcard = false;

char       **user_name;
int         *user_id;
int         *face;
int         *finger;
char       **role;
uint16_t     user_len = 0;
portMUX_TYPE spi_mux  = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE i2c_mux  = portMUX_INITIALIZER_UNLOCKED;