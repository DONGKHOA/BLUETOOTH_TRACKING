idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_timer driver
                    PRIV_REQUIRES APP_FingerPrint)
