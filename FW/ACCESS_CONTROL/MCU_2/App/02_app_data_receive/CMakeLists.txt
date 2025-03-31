idf_component_register(SRCS "app_data_receive.c"
                       INCLUDE_DIRS "."
                       REQUIRES CAN 16_app_data SN65HVD230DR Env
                       )