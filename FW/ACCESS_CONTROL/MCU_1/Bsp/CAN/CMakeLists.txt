set(component_srcs "can.c")

idf_component_register(SRCS "${component_srcs}"
                        INCLUDE_DIRS "."
                        REQUIRES "driver"
                        )