#ifndef SERVER_H
#define SERVER_H

#include "esp_http_server.h"
#include "esp_camera.h"

extern const char* WEBPAGE_HTML; ///< HTML content for the main web page.
extern const char* GESTURES[]; ///< Array of gesture names corresponding to model output classes.

/**
 * @brief HTTP request handler for the main page.
 *
 * @param req The HTTP request.
 * @return ESP_OK on success, ESP HTTP errors on failure.
 */
esp_err_t index_handler(httpd_req_t *req);

/**
 * @brief Starts the web server.
 *
 * @param[out] server The HTTP server handle.
 * @param[in] model_ctx A pointer to the TFLiteModel to be used by handlers.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t startServer(httpd_handle_t &server, void* model_ctx);

/**
 * @brief Captures a photo from the camera or a mock source.
 *
 * @param[out] fb Pointer to the camera frame buffer structure to hold the captured image.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
camera_fb_t* get_photo();

/**
 * @brief Frees the resources associated with a captured photo.
 *
 * @param[in] fb Pointer to the camera frame buffer structure to be freed.
 */
void free_resource(camera_fb_t *fb);

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
esp_err_t capture_handler(httpd_req_t *req);

/**
 * @brief HTTP request handler for retrieving the detected gesture name.
 *
 * This function is called when a GET request is made to the /gesture_name URI.
 * It retrieves the name of the detected gesture from the TFLite model and
 * sends it back to the client as plain text.
 *
 * @param req The HTTP request.
 * @return ESP_OK on success, or ESP_FAIL on failure.
 */
esp_err_t gesture_name_handler(httpd_req_t *req);

#endif // SERVER_H