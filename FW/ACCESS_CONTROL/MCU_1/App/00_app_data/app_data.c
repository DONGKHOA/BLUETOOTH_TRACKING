/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "app_data.h"

/******************************************************************************
 *   PUBLIC DATA
 *****************************************************************************/

DATA_System_t s_data_system;

uint8_t enroll_number_id_send;
uint8_t enroll_index;
char    enroll_number_id[64] = { 0 };
char    full_name[64] = "";
int     full_name_len = 0;
int     packet_count  = 0;