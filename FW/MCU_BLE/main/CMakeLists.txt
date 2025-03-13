set (includes ".")

set (sources "main.c")

set (requires 00_app_ble_ibeacon 
              01_app_ble_tracking 
              02_app_data_receive 
              03_app_data_transmit 
              04_app_face_recognition 
              05_app_gui 
              06_app_handle_camera
              07_app_led_status
              08_app_data
              lvgl 
              driver
              SN65HVD230DR
              ILI9341
              SPI
              NVS
              GPIO)

idf_component_register(SRCS ${sources}
                       INCLUDE_DIRS ${includes}
                       REQUIRES ${requires})