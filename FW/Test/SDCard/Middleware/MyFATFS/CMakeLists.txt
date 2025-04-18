idf_component_register(SRCS "ff.c" "diskio.c" "ffsystem.c" "ffunicode.c"
                       INCLUDE_DIRS "."
                       REQUIRES main SDCard
                       PRIV_REQUIRES SPI)
