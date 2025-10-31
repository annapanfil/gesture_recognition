import cv2

im = cv2.imread('../data/HG14/HG14-Hand-Gesture/Gesture_2/2000.jpg')

# convert to grayscale 96x96
im_gray = cv2.cvtColor(im, cv2.COLOR_BGR2GRAY)
im_resized = cv2.resize(im_gray, (96, 96))

# dump to a c-style array
with open('../include/mock_image.h', 'w') as f:
    f.write("#ifndef MOCK_IMAGE\n")
    f.write("#define MOCK_IMAGE\n\n")
    f.write("#include <stdint.h>\n\n")
    f.write("extern const uint8_t mock_image[96*96];\n\n")
    f.write("extern const uint8_t mock_image_size;\n")
    f.write("#endif // MOCK_IMAGE\n")

with open('../main/mock_image.c', 'w') as f:
    f.write('#include "mock_image.h"\n\n')
    f.write('const uint8_t mock_image[96*96] = {\n')

    for row in im_resized:
        for pixel in row:
            f.write(f'{pixel}, ')
        f.write('\n')
    f.write('};\n')

    f.write('const uint8_t mock_image_size = 96;\n')