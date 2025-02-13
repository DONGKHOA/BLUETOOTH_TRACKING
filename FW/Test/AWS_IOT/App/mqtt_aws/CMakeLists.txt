idf_component_register(SRCS app_mqtt_aws.c
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES "freertos" "coreMQTT" "backoffAlgorithm" "posix_compat" "esp_secure_cert_mgr"
                       )