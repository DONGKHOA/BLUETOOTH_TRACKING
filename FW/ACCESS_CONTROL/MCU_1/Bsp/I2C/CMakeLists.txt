set(component_srcs "i2c.c")

idf_component_register(SRCS "${component_srcs}"
                        INCLUDE_DIRS "."
                        REQUIRES driver
                        )