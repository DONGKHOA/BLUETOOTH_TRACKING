idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES ble_ibeacon NVS wifi_connect UART mqtt_client)
