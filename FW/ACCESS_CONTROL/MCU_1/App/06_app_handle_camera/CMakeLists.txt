idf_component_register(SRCS "app_handle_camera.c"
                       INCLUDE_DIRS "."
                       REQUIRES 00_app_data
                       PRIV_REQUIRES espressif__esp32-camera "freertos"
                       )