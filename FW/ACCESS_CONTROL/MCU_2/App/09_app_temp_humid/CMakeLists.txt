idf_component_register(SRCS "app_temp_humid.c"
                       INCLUDE_DIRS "."
                       REQUIRES 00_app_data DHT22
                       )