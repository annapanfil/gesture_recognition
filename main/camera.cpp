#include "camera.h"

#include "esp_heap_caps.h"
#include "esp_system.h"

static const char* TAG = "camera";

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