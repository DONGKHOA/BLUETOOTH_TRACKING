idf_component_register(SRCS "app_time_system.c"
                       INCLUDE_DIRS "."
                       REQUIRES DS3231 00_app_data WiFi
                       )