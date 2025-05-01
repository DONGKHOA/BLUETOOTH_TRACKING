set (includes ".")

set (sources "main.c")

set (requires 00_app_data
              01_app_fingerprint
              02_app_data_receive 
              03_app_data_transmit 
              04_app_local_database
              05_app_rtc
              06_app_handle_wifi
              07_app_mqtt_client
              08_app_led_status
              09_app_timestamp
              10_app_configuration
              11_app_control_sdcard
              driver
              SN65HVD230DR
              AS608
              SPI
              NVS
              UART
              GPIO)

idf_component_register(SRCS ${sources}
                       INCLUDE_DIRS ${includes}
                       REQUIRES ${requires})