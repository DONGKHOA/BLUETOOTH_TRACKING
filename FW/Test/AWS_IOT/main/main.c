#include <stdio.h>

#include "wifi.h"
#include "nvs_rw.h"

#include "app_mqtt_aws.h"

// static uint8_t ssid[] = "PIF_CLUB";
// static uint8_t pass[] = "chinsochin";

static uint8_t ssid[] = "Thuc Coffee";
static uint8_t pass[] = "18006230";

void app_main(void)
{
    NVS_Init();
    WIFI_StaInit();
    WIFI_Connect(ssid, pass);

    aws_iot_demo_main(0, NULL);
}
