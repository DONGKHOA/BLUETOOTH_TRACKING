idf_component_register(SRCS "app_control_sdcard.c"
                       INCLUDE_DIRS "."
                       REQUIRES SPI 00_app_data Env MyFATFS
                       )