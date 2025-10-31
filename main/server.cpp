#include "camera.h"
#include "server.h"

#ifdef CONFIG_ENABLE_QEMU_DEBUG
#include "camera_mock.h"
#endif //CONFIG_ENABLE_QEMU_DEBUG

#include "esp_http_server.h"
#include "esp_log.h"
#include "tflite_model.h"
#include "esp_netif.h"
#include <memory>

static const char* TAG = "server";

const char *MAIN_PAGE = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32-CAM Gesture Detection</title>
<style>
    body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; margin: 0; padding: 20px; }
    h1 { color: #333; }
    #capture-btn { padding: 10px 20px; font-size: 16px; margin: 20px; cursor: pointer; }
    .card { display: inline-block; text-align: center; background: #fff; padding: 15px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.2); margin-top: 20px; }
    .card img { max-width: 80%; border-radius: 8px;  margin: 0 auto 10px auto;}
    .gesture-name { font-size: 20px; color: #007BFF; margin-top: 10px; display: block; }
</style>
</head>
<body>
<h1>ESP32-CAM Gesture Detection</h1>

<div id="result" class="card">
    <img id="captured-img" src="" alt="Captured image" style="display:none;">
    <span id="gesture-name" class="gesture-name"></span>
    <button id="capture-btn" onclick="capture()">Capture & Detect</button>
</div>

<script>
async function capture() {
    const response = await fetch('/capture');
    const blob = await response.blob();
    const img = document.getElementById('captured-img');
    img.src = URL.createObjectURL(blob);
    img.onload = () => {
        img.width = img.naturalWidth * 2; // scale by 2 for better visibility
        img.height = img.naturalHeight * 2;
    };
    img.style.display = 'block';

    // fetch gesture name
    const gestureResponse = await fetch('/gesture_name'); // endpoint returning detected class
    const gestureName = await gestureResponse.text();
    document.getElementById('gesture-name').textContent = `Detected gesture: ${gestureName}`;
}
</script>
</body>
</html>
)rawliteral";


const char* GESTURES[] = {"fist", "1 finger", "2 fingers", "3 fingers", "4 fingers", "palm", "phone", "mouth", "open mouth", "ok", "pinky", "rock1", "rock2", "stop"};

esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, MAIN_PAGE, strlen(MAIN_PAGE));
}

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

        httpd_uri_t gesture_name_uri = {
            .uri = "/gesture_name",
            .method = HTTP_GET,
            .handler = gesture_name_handler,
            .user_ctx = model_ctx};

        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &capture_uri);
        httpd_register_uri_handler(server, &gesture_name_uri);
        return ESP_OK;
    } else {
        return ESP_FAIL;
    }
}

camera_fb_t* get_photo() {
    #ifndef CONFIG_ENABLE_QEMU_DEBUG
    return esp_camera_fb_get();
    #else
    return mock_get_photo();
    #endif //CONFIG_ENABLE_QEMU_DEBUG 
}

void free_resource(camera_fb_t *fb){
    #ifndef CONFIG_ENABLE_QEMU_DEBUG
    esp_camera_fb_return(fb);
    #else
    mock_free_resource(fb);
    #endif //CONFIG_ENABLE_QEMU_DEBUG 
}

esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t *fb = get_photo();
    if (!fb) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Retrieve the model from the user context
    TFLiteModel* model = static_cast<TFLiteModel*>(req->user_ctx);
    if (!model || !model->is_initialized()) {
        ESP_LOGE(TAG, "Model not initialized or not passed in context");
        httpd_resp_send_500(req);
        free_resource(fb);
        return ESP_FAIL;
    }

    // Get model input tensor and dimensions
    // Could add validation in capture_handler():
    if (model->input()->type != kTfLiteFloat32) {
        ESP_LOGE(TAG, "Wrong input tensor type");
        return ESP_FAIL;
    }
    
    float *model_input = model->input()->data.f;
    
    TfLiteIntArray* input_dims = model->input()->dims;
    size_t model_input_width = input_dims->data[2];
    size_t model_input_height = input_dims->data[3];

    ESP_LOGI(TAG, "Model input dims %d %d", model_input_width, model_input_height);
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
        
        model->set_last_detected_index(argmax);

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

    free_resource(fb);
    ESP_LOGI(TAG, "Camera: handle capture request");

    return ESP_OK;
}

esp_err_t gesture_name_handler(httpd_req_t *req) {
    TFLiteModel* model = static_cast<TFLiteModel*>(req->user_ctx);
    if (!model || !model->is_initialized()) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    const char* gesture = GESTURES[model->last_detected_index()];
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, gesture);
    return ESP_OK;
}
