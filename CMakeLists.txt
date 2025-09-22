cmake_minimum_required(VERSION 3.10)

project(GesturesRecognition)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)

add_executable(gestures src/main.cpp)
target_link_libraries(gestures ${OpenCV_LIBS})