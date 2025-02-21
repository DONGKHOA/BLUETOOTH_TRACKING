set(component_srcs "uart.c")

idf_component_register(SRCS "${component_srcs}"
                        INCLUDE_DIRS "."
                        REQUIRES "driver" EXTI
                        )