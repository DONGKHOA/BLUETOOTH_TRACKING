idf_component_register(SRCS "app_local_database.c"
                       INCLUDE_DIRS "."
                       REQUIRES SPI 00_app_data Env MyFATFS
                       )