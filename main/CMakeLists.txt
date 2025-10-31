idf_component_register(SRCS "ethernet.cpp" "server.cpp" "camera.cpp" "wifi.cpp" "main.cpp" "tflite_model.cpp" "../models/model.cc"
                        INCLUDE_DIRS "../include"
                        REQUIRES esp_http_server esp_wifi nvs_flash esp_event esp_netif wifi_provisioning esp_eth)

target_compile_options(${COMPONENT_LIB} PRIVATE "-fno-common")

project(gestures)
add_custom_target(run-qemu
    COMMAND esptool.py --chip esp32 merge_bin --output result.bin --fill-flash-size 2MB 0x1000 bootloader/bootloader.bin 0x8000 partition_table/partition-table.bin 0x10000 ${PROJECT_NAME}.bin --flash_mode dio --flash_freq 40m --flash_size 2MB
    COMMAND qemu-system-xtensa -nographic -machine esp32 -drive file=result.bin,if=mtd,format=raw -m 8M -nic user,model=open_eth,hostfwd=tcp::8080-:80
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    USES_TERMINAL
)