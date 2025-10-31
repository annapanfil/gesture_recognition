#ifndef CAMERA_MOCK_H
#define CAMERA_MOCK_H

#include "esp_camera.h"

/**
 * @brief Mock function to simulate capturing a photo.
 * @return Pointer to a mock camera frame buffer structure.
 */
camera_fb_t *mock_get_photo();

/**
 * @brief Mock function to free resources associated with a camera frame buffer.
 * @param fb Pointer to the camera frame buffer structure to be freed.
 */
void mock_free_resource(camera_fb_t *fb);

#endif // CAMERA_MOCK_H

