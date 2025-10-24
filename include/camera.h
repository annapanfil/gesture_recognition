#ifndef CAMERA_H
#define CAMERA_H

#include "esp_log.h"
#include "esp_err.h"
#include "esp_camera.h"


#include <memory>


/**
 * @name Camera GPIO Pin Definitions
 * @brief GPIO pin assignments for the ESP32-CAM camera module.
 * @{ 
 */
#define PWDN_GPIO_NUM  32 ///< Power Down Pin
#define RESET_GPIO_NUM -1 ///< Reset Pin (set to -1 if not used)
#define XCLK_GPIO_NUM  0  ///< System Clock Pin
#define SIOD_GPIO_NUM  26 ///< SCCB Data Pin
#define SIOC_GPIO_NUM  27 ///< SCCB Clock Pin

#define Y9_GPIO_NUM    35 ///< D9 Data Pin
#define Y8_GPIO_NUM    34 ///< D8 Data Pin
#define Y7_GPIO_NUM    39 ///< D7 Data Pin
#define Y6_GPIO_NUM    36 ///< D6 Data Pin
#define Y5_GPIO_NUM    21 ///< D5 Data Pin
#define Y4_GPIO_NUM    19 ///< D4 Data Pin
#define Y3_GPIO_NUM    18 ///< D3 Data Pin
#define Y2_GPIO_NUM    5  ///< D2 Data Pin
#define VSYNC_GPIO_NUM 25 ///< VSYNC Pin
#define HREF_GPIO_NUM  23 ///< HREF Pin
#define PCLK_GPIO_NUM  22 ///< Pixel Clock Pin
/** @} */ // End of Camera GPIO Pin Definitions


/**
 * @brief Configures and initializes the camera.
 *
 * Sets the camera pins, pixel format, frame size, and other parameters. 
 * Corrects image orientation.
 * @return ESP_OK on success, ESP_FAIL on failure.
 */
esp_err_t initCamera();


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
                                   float *dst, int dst_w, int dst_h);

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
std::unique_ptr<camera_fb_t, CameraFbDeleter> convert_grayscale_to_jpeg(camera_fb_t *grayscale_fb);
#endif // CAMERA_H
