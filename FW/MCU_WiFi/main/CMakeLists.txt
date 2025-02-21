idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES MyFATFS esp_timer driver
                    PRIV_REQUIRES SPI DHT22 SDCard_SPI ADS1115 MCP4822 SN65HVD230DR APP_ReadData APP_RTC APP_FingerPrint RS485)
