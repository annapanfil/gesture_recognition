#ifndef WEB_GUI_H
#define WEB_GUI_H

#include "esp_http_server.h"

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

#endif // WEB_GUI_H