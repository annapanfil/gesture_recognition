cmake_minimum_required(VERSION 3.10)

project(GesturesRecognition)

set(CMAKE_CXX_STANDARD 17)

set(Torch_DIR "/home/anna/libs/libtorch/share/cmake/Torch")
find_package(OpenCV REQUIRED)
find_package(Torch REQUIRED)
find_package(Threads REQUIRED) # pthread

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_executable(gestures src/main.cpp src/cnn.cpp src/gesture_dataset.cpp)
target_link_libraries(gestures ${OpenCV_LIBS} ${TORCH_LIBRARIES} Threads::Threads)