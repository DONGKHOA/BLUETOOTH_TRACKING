/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include "wifi_scan.h"
#include "wifi_ap.h"
#include "wifi_sta.h"
#include "wifi_smart_config.h"
#include "wifi_store.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC DEFINES
   ***************************************************************************/

#define SCAN_LIST_SIZE   50
#define LIMIT_STORE_WIFI 10
#define MAXIMUM_RETRY    10

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/
  typedef enum
  {
    CONNECT_OK = 1,
    CONNECT_FAIL,
    UNEXPECTED_EVENT,
  } WIFI_Status_t;

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  WIFI_Status_t WIFI_state_connect(void);

#ifdef __cplusplus
}
#endif