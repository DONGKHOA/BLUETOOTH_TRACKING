idf_component_register(SRCS "app_face_recognition.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES 00_app_data esp-dl
                       PRIV_REQUIRES espressif__esp32-camera "freertos"
                       )