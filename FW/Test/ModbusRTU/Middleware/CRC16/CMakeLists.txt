set(component_srcs "crc16.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       )