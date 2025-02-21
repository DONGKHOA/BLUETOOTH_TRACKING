idf_component_register(SRCS "ff.c" "diskio.c" "ffsystem.c" "ffunicode.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES SPI SDCard_SPI)
