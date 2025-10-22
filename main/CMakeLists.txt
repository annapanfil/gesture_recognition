idf_component_register(SRCS "wifi.cpp" "main.cpp" "tflite_model.cpp" "../models/model.cc"
                        INCLUDE_DIRS "../include"
                        REQUIRES esp_http_server esp_wifi nvs_flash esp_event esp_netif wifi_provisioning)

target_compile_options(${COMPONENT_LIB} PRIVATE "-fno-common")