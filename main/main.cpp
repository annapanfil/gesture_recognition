#include "esp_camera.h" //from ESP32 Camera
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"


#include "consts.h"

const char* TAG = "gestures";

void initCamera(){
    ESP_LOGI(TAG, "Camera: Initializing...");

    // Check if PSRAM is available
    size_t total_heap = esp_get_free_heap_size();
    ESP_LOGI(TAG, "Total free heap: %d bytes (if > 250 000 we likely have PSRAM)", total_heap);

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
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QQVGA; //160x120
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY; //CAMERA_GRAB_LATEST;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    if (esp_camera_init(&config) != ESP_OK) {
        ESP_LOGE(TAG, "Camera: Failed to initialize");
        return;
    }

    // fix upside down image
    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);

    ESP_LOGI(TAG, "Camera: Initialized successfully");
}


esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_send(req, (const char *)fb->buf, fb->len);

    esp_camera_fb_return(fb);
    ESP_LOGI(TAG, "Camera: handle capture request");

    return ESP_OK;
}


// Simple HTML page for testing
static const char* MAIN_PAGE = R"rawliteral(
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

esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, MAIN_PAGE, strlen(MAIN_PAGE));
}

void startServer(httpd_handle_t& server) {
    ESP_LOGI(TAG, "Wifi: Starting server...");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
            httpd_uri_t index_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = index_handler,
            .user_ctx = NULL
        };

        httpd_uri_t capture_uri = {
            .uri = "/capture",
            .method = HTTP_GET,
            .handler = capture_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &capture_uri);
        
        ESP_LOGI(TAG, "Server started successfully");
    }
    else {
        ESP_LOGE(TAG, "Failed to start server");
    }
}

int init_wifi() {
    // Inicjalizacja NVS – non volatile storage for wifi settings storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
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
    strcpy((char*)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wifi: Connecting to Wi-Fi...");

    // Connect to Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Wait for connection
    int retries = 0;
    while (retries < 20) {  // 20 second timeout
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info) == ESP_OK) {
            if (ip_info.ip.addr != 0) {
                ESP_LOGI(TAG, "WiFi connected! IP: " IPSTR, IP2STR(&ip_info.ip));
                return 0;
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        retries++;
    }
    
    ESP_LOGE(TAG, "WiFi connection failed");
    return -1;
}


int main() {
    httpd_handle_t server = NULL;
    initCamera();

    // init WiFi
    init_wifi();
    startServer(server);

    ESP_LOGI(TAG, "Setup complete");
    return 0;
}


extern "C" void app_main() {
    main();
}