set (includes ".")

set (sources "main.c")

set (requires 01_app_ble_ibeacon 
              02_app_ble_tracking 
              03_app_led_status
              06_app_handle_wifi
              04_app_mqtt_client
              05_app_timestamp
              00_app_data
              driver
              NVS
              GPIO)

idf_component_register(SRCS ${sources}
                       INCLUDE_DIRS ${includes}
                       REQUIRES ${requires})