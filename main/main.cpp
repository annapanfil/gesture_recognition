
/**
 * @file main.cpp
 * @brief Main application file for the ESP32 gesture recognition project.
 *
 * This file contains the main application logic for the ESP32-CAM based gesture
 * recognition system. It initializes the camera, Wi-Fi, and the TensorFlow Lite
 * model. It then starts a web server that provides a web interface for
 * capturing images and viewing the recognized gestures.
 */

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_system.h"

#include "model.h"
#include "wifi.h"
#include "camera.h"
#include "web_gui.h"
#include "tflite_model.h"

/**
 * @brief Logging tag for ESP_LOGx macros.
 */
static const char* TAG = "gestures";

/**
 * @brief Print memory statistics to the console.
 *
 * Log the total, free, and used heap sizes for both internal
 * RAM and PSRAM. Also logs the minimum free heap size and the largest free
 * block size. This is useful for debugging memory issues.
 */
void print_memory_stats() {
    ESP_LOGI("MEMORY", "=== Memory Stats ===");

    // Internal RAM (DRAM)
    size_t total_internal = heap_caps_get_total_size(MALLOC_CAP_8BIT);
    size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t min_free_internal = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
    size_t largest_block_internal = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    ESP_LOGI("MEMORY", "Total Heap: %zu bytes", total_internal);
    ESP_LOGI("MEMORY", "Free Heap: %zu bytes", free_internal);
    ESP_LOGI("MEMORY", "Used Heap: %zu bytes", total_internal - free_internal);
    ESP_LOGI("MEMORY", "Largest Free Block: %zu bytes", largest_block_internal);
    ESP_LOGI("MEMORY", "Minimum Free Heap Ever: %zu bytes", min_free_internal);

    // PSRAM
    size_t total_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t largest_block_psram = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);

    ESP_LOGI("MEMORY", "Total PSRAM: %zu bytes", total_psram);
    ESP_LOGI("MEMORY", "Free PSRAM: %zu bytes", free_psram);
    ESP_LOGI("MEMORY", "Largest Free PSRAM Block: %zu bytes", largest_block_psram);

    // Task info
    ESP_LOGI("MEMORY", "Main Task Stack High Water Mark: %zu bytes",
             uxTaskGetStackHighWaterMark(NULL));

    ESP_LOGI("MEMORY", "====================");
}

/**
 * @brief The main function of the application.
 *
 * This function initializes the camera, Wi-Fi, and the TFLite model. It then
 * starts the web server and suspends the task.
 *
 * @return 0 on success, -1 on failure.
 */
int main() {
    ESP_LOGI(TAG, "Initialising...");
    
    if (initCamera() != ESP_OK ) {
        ESP_LOGE(TAG, "Camera initialization failed.");
        return -1;
    }

    WifiManager::initialize();
    WifiManager& wifi_mgr = WifiManager::getInstance();

    wifi_mgr.wifi_hw_init();
    wifi_mgr.prov_start();
    
    auto tflite_model = std::make_unique<TFLiteModel>(model_tflite, &model_tflite_len);
    
    if (!tflite_model->init()) {
        ESP_LOGE(TAG, "Failed to initialize TFLite model");
        return -1;
    }
    

    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    if (!wifi_mgr.wait_for_connection(30000)) { // 30s timeout
        ESP_LOGE(TAG, "WiFi connection timeout");
        return -1;
    }

    httpd_handle_t server = NULL;
    if (startServer(server, tflite_model.get()) != ESP_OK){
        ESP_LOGI(TAG, "Failed to start server");
        return -1;
    }
    
    ESP_LOGI(TAG, "Setup complete");

    // Suspend the main task, as all operations are handled by event loops and other tasks
    vTaskSuspend(NULL);
    return 0;
}

// Entry point for the ESP-IDF application
extern "C" void app_main() { main(); }
