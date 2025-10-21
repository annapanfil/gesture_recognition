
/**
 * @file main.cpp
 * @brief Main application file for the ESP32 gesture recognition project.
 *
 * This file contains the main application logic for the ESP32-CAM based gesture
 * recognition system. It initializes the camera, Wi-Fi, and the TensorFlow Lite
 * model. It then starts a web server that provides a web interface for
 * capturing images and viewing the recognized gestures.
 */

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "model.h"
#include "consts.h"
#include "tflite_model.h"

#include "esp_system.h"
#include "esp_heap_caps.h"

#include <memory>

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
 * @brief Configures and initializes the camera.
 *
 * Sets the camera pins, pixel format, frame size, and other parameters. 
 * Corrects image orientation.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t initCamera() {
    ESP_LOGI(TAG, "Camera: Initializing...");

    // Check if PSRAM is available
    size_t total_heap = esp_get_free_heap_size();
    ESP_LOGI(TAG,
             "Total free heap: %d bytes (if > 250 000 we likely have PSRAM)",
             total_heap);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_GRAYSCALE;
    config.frame_size = FRAMESIZE_96X96;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    if (esp_camera_init(&config) != ESP_OK) {
        ESP_LOGE(TAG, "Camera: Failed to initialize");
        return ESP_FAIL;
    }

    // Fix upside down image
    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);

    return ESP_OK;
}

/**
 * @brief Resizes and normalizes a grayscale image.
 *
 * Takes a grayscale image, resizes it to the specified
 * dimensions, and normalizes the pixel values to the range [0, 1].
 *
 * @param src The source image buffer.
 * @param src_w The width of the source image.
 * @param src_h The height of the source image.
 * @param[out] dst The destination buffer for the resized and normalized image.
 * @param dst_w The desired width of the destination image.
 * @param dst_h The desired height of the destination image.
 */
void resize_and_normalize_grayscale(uint8_t *src, int src_w, int src_h,
                                   float *dst, int dst_w, int dst_h) {
    float scale_x = (float)src_w / dst_w;
    float scale_y = (float)src_h / dst_h;

    for (int y = 0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            int src_x = (int)(x * scale_x);
            int src_y = (int)(y * scale_y);
            uint8_t pixel = src[src_y * src_w + src_x];
            dst[y * dst_w + x] = pixel / 255.0f; // Normalize to 0-1
        }
    }
}

/**
 * Custom deleter to ensure both the struct and buffer are freed
 */ 
struct CameraFbDeleter{
    void operator()(camera_fb_t* fb) const {
        if (fb) {
            if (fb->buf) {
                free(fb->buf); // Use free for buffer allocated by frame2jpg
            }
            delete fb;
        }
    }
};

/**
 * @brief Converts a grayscale framebuffer to a JPEG framebuffer.
 *
 * @param grayscale_fb The grayscale framebuffer to convert.
 * @return An unique pointer to a new framebuffer containing the JPEG image, or nullptr on failure.
 */
std::unique_ptr<camera_fb_t, CameraFbDeleter> convert_grayscale_to_jpeg(camera_fb_t *grayscale_fb) {
    if (grayscale_fb->format != PIXFORMAT_GRAYSCALE) {
        return nullptr;
    }

    size_t jpeg_size = 0;
    uint8_t *jpeg_buf = NULL;

    bool jpeg_converted = frame2jpg(grayscale_fb, 80, &jpeg_buf, &jpeg_size);

    if (jpeg_converted && jpeg_buf) {
        // Create new frame buffer with JPG photo
        std::unique_ptr<camera_fb_t, CameraFbDeleter> jpeg_fb(new camera_fb_t); 
        jpeg_fb->buf = jpeg_buf;
        jpeg_fb->len = jpeg_size;
        jpeg_fb->width = grayscale_fb->width;
        jpeg_fb->height = grayscale_fb->height;
        jpeg_fb->format = PIXFORMAT_JPEG;
        return jpeg_fb;
    }

    return nullptr;
}

/**
 * @brief HTTP request handler for capturing an image and performing gesture
 * recognition.
 *
 * This function is called when a GET request is made to the /capture URI. It
 * captures an image from the camera, preprocesses it, runs inference with the
 * TFLite model, and sends the resulting image (either the original or the
 * preprocessed one) back to the client as a JPEG.
 *
 * @param req The HTTP request.
 * @return ESP_OK on success, or ESP_FAIL on failure.
 */
esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Retrieve the model from the user context
    TFLiteModel* model = static_cast<TFLiteModel*>(req->user_ctx);
    if (!model || !model->is_initialized()) {
        ESP_LOGE(TAG, "Model not initialized or not passed in context");
        httpd_resp_send_500(req);
        esp_camera_fb_return(fb);
        return ESP_FAIL;
    }

    // Get model input tensor and dimensions
    float *model_input = model->input()->data.f;
    
    TfLiteIntArray* input_dims = model->input()->dims;
    size_t model_input_width = input_dims->data[1];
    size_t model_input_height = input_dims->data[2];

    resize_and_normalize_grayscale(fb->buf, fb->width, fb->height, model_input,
                                   model_input_width, model_input_height);

    // Predict gesture
    if (model->invoke() != kTfLiteOk) {
        ESP_LOGE(TAG, "Cannot invoke interpreter");
    } else {
        TfLiteTensor *output = model->output();
        float *logits = output->data.f;

        float max = logits[0];
        uint argmax = 0;

        for (int i = 0; i < output->dims->data[1]; i++) {
            ESP_LOGI(TAG, "Logit %d: %f", i, logits[i]);
            if (logits[i] > max) {
                max = logits[i];
                argmax = i;
            }
        }

        ESP_LOGI(TAG, "DETECTED GESTURE: %s", GESTURES[argmax]);
    }

    std::unique_ptr<camera_fb_t, CameraFbDeleter> jpeg_fb = nullptr;
    
    // Display in the web GUI
    jpeg_fb = convert_grayscale_to_jpeg(fb);
    
    if (!jpeg_fb) {
        ESP_LOGE(TAG, "Failed to convert to JPEG");
        httpd_resp_send_500(req);
    } else {
        httpd_resp_set_type(req, "image/jpeg");
        httpd_resp_send(req, (const char *)jpeg_fb->buf, jpeg_fb->len);
    }

    esp_camera_fb_return(fb);
    ESP_LOGI(TAG, "Camera: handle capture request");

    return ESP_OK;
}

// Simple HTML page for testing
static const char *MAIN_PAGE = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-CAM Gesture Detection</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
    <h1>Gesture Detection</h1>
    <button onclick="capture()">Capture & Detect</button>
    <div id="result"></div>
    <script>
        async function capture() {
            const response = await fetch('/capture');
            const blob = await response.blob();
            const resultDiv = document.getElementById('result');
            resultDiv.innerHTML = `<img src="${URL.createObjectURL(blob)}" style="max-width: 100%">`;
        }
    </script>
</body>
</html>
)rawliteral";

/**
 * @brief HTTP request handler for the main page.
 *
 * @param req The HTTP request.
 * @return ESP_OK on success, ESP HTTP errors on failure.
 */
esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, MAIN_PAGE, strlen(MAIN_PAGE));
}

/**
 * @brief Starts the web server.
 *
 * @param[out] server The HTTP server handle.
 * @param[in] model_ctx A pointer to the TFLiteModel to be used by handlers.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t startServer(httpd_handle_t &server, void* model_ctx) {
    ESP_LOGI(TAG, "Wifi: Starting server...");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t index_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = index_handler,
            .user_ctx = NULL};

        httpd_uri_t capture_uri = {
            .uri = "/capture",
            .method = HTTP_GET,
            .handler = capture_handler,
            .user_ctx = model_ctx};

        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &capture_uri);
        return ESP_OK;
    } else {
        return ESP_FAIL;
    }
}

/**
 * @brief Initializes the Wi-Fi connection.
 *
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t init_wifi() {
    // Initialize NVS (non-volatile storage) for Wi-Fi settings
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP stack and event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Configure Wi-Fi interface
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strcpy((char *)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wifi: Connecting to Wi-Fi...");

    // Connect to Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Wait for connection
    int retries = 0;
    while (retries < 20) { // 20 second timeout
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(
                esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"),
                &ip_info) == ESP_OK) {
            if (ip_info.ip.addr != 0) {
                ESP_LOGI(TAG, "WiFi connected! IP: " IPSTR,
                         IP2STR(&ip_info.ip));
                return ESP_OK;
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        retries++;
    }
    return ESP_FAIL;
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
    
    if (initCamera() != ESP_OK){
        ESP_LOGI(TAG, "Failed to intitialise camera");
        return -1;
    }

    if (init_wifi() != ESP_OK) {
        ESP_LOGE(TAG, "WiFi connection failed");
        return -1;
    }
    
    auto tflite_model = std::make_unique<TFLiteModel>(model_tflite, &model_tflite_len);
    
    if (!tflite_model->init()) {
        ESP_LOGE(TAG, "Failed to initialize TFLite model");
        return -1;
    }
    
    httpd_handle_t server = NULL;
    if (startServer(server, tflite_model.get()) != ESP_OK){
        ESP_LOGI(TAG, "Failed to start server");
    }
    
    ESP_LOGI(TAG, "Setup complete");

    // Suspend the main task, as all operations are handled by event loops and other tasks
    vTaskSuspend(NULL);
    return 0;
}

// Entry point for the ESP-IDF application
extern "C" void app_main() { main(); }
