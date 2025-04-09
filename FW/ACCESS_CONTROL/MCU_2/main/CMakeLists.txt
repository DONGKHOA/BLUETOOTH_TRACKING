set (includes ".")

set (sources "main.c")

set (requires 01_app_fingerprint
              02_app_data_receive 
              03_app_data_transmit 
              05_app_local_database
              06_app_analog_output
              08_app_rtc
              09_app_analog_input
              10_app_digital_in_out
              11_app_handle_wifi
              12_app_handle_modbusRTU
              13_app_mqtt_client
              14_app_led_status
              15_app_timestamp
              00_app_data
              17_app_configuration
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