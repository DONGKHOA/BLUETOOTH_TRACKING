set(component_srcs "button.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       REQUIRES GPIO EXTI 
                       PRIV_REQUIRES "freertos"
                       )