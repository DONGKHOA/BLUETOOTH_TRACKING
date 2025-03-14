idf_component_register(SRCS "app_data_transmit.c"
                       INCLUDE_DIRS "."
                       REQUIRES CAN 08_app_data SN65HVD230DR
                       )