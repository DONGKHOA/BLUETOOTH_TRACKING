idf_component_register(SRCS "app_handle_wifi.c"
                       INCLUDE_DIRS "."
                       REQUIRES WiFi 16_app_data Env
                       )