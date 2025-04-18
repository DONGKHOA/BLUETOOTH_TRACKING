idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    PRIV_REQUIRES esp_timer ADS1115)
