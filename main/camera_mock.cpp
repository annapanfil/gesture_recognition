#include "mock_image.h"
#include "camera_mock.h"
#include "esp_log.h"

camera_fb_t* mock_get_photo() {
    camera_fb_t* fb = new camera_fb_t;
    fb->buf = mock_image;
    fb->len = mock_image_size * mock_image_size;
    fb->width = mock_image_size;
    fb->height = mock_image_size;
    fb->format = PIXFORMAT_GRAYSCALE;

    return fb;
}

void mock_free_resource(camera_fb_t* fb) {
    delete fb;
}