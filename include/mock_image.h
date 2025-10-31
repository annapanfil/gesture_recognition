#ifndef MOCK_IMAGE
#define MOCK_IMAGE

#include <stdint.h>

extern uint8_t mock_image[96*96]; ///< Mock grayscale image data from the dataset

extern unsigned int mock_image_size; ///< Size of one side of the mock image (assuming square)
#endif // MOCK_IMAGE
