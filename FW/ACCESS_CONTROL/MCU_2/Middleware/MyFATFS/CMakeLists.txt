idf_component_register(SRCS "ff.c" "diskio.c" "ffsystem.c" "ffunicode.c"
                       INCLUDE_DIRS "."
                       REQUIRES SDCard 00_app_data
                       PRIV_REQUIRES SPI)
