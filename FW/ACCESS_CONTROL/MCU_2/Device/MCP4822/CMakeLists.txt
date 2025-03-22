set(component_srcs "mcp4822.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       REQUIRES driver SPI GPIO
                       )