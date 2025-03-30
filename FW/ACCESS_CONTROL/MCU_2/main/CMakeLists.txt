set (includes ".")

set (sources "main.c")

set (requires 00_app_security
              01_app_fingerprint
              02_app_data_receive 
              03_app_data_transmit 
              04_app_spi_gate_keeper
              05_app_sd_card
              06_app_analog_output
              07_app_i2c_gate_keeper
              08_app_rtc
              09_app_analog_input
              10_app_digital_in_out
              11_app_handle_wifi
              12_app_handle_modbusRTU
              13_app_mqtt_client
              14_app_led_status
              15_app_timestamp
              16_app_data
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