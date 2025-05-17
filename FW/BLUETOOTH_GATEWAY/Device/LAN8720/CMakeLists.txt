idf_component_register(
    SRCS "lan8720.c"
    INCLUDE_DIRS "."
    REQUIRES esp_eth
    PRIV_REQUIRES "freertos"
)