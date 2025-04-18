set (includes "AP_Mode"
              "STA_Mode"
              "Scan"
              "Store"
              "Smart_Config"
              ".")

set (sources  "Scan/wifi_scan.c"

              "STA_Mode/wifi_sta.c"
              "AP_Mode/wifi_ap.c"
              "Store/wifi_store.c"
              "Smart_Config/wifi_smart_config.c"
              "wifi_helper.c")

set (requires "freertos" 
              "lwip" 
              "esp_wifi"
              NVS)

idf_component_register(SRCS ${sources}
                       INCLUDE_DIRS ${includes}
                       REQUIRES ${requires})