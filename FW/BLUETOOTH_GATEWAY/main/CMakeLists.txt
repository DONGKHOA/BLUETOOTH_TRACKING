set (includes ".")

set (sources "main.c")

set (requires 00_app_data
              01_app_ble_ibeacon 
              02_app_ble_tracking 
              03_app_led_status
              04_app_mqtt_client
              06_app_handle_wifi
              07_app_configuration
              08_app_user_button
              10_app_display
              driver
              NVS
              I2C
              GPIO
              EXTI
              UART)

idf_component_register(SRCS ${sources}
                       INCLUDE_DIRS ${includes}
                       REQUIRES ${requires})