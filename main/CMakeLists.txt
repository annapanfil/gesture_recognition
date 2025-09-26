idf_component_register(SRCS "main.cpp"
                        INCLUDE_DIRS "../include"
                        REQUIRES esp_http_server esp_wifi nvs_flash esp_event esp_netif)