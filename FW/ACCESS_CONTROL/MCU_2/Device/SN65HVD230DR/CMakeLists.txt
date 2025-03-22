set(component_srcs "sn65hvd230dr.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       REQUIRES CAN)