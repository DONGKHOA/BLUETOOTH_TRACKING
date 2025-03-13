set (includes ".")

set (sources "main.c")

set (requires 0_app_ble_ibeacon 
              1_app_ble_tracking 
              2_app_data_receive 
              3_app_data_transmit 
              4_app_face_recognition 
              5_app_gui 
              6_app_handle_camera 
              7_app_data
              lvgl 
              driver
              SN65HVD230DR
              ILI9341
              SPI
              GPIO)

idf_component_register(SRCS ${sources}
                       INCLUDE_DIRS ${includes}
                       REQUIRES ${requires})