cmake_minimum_required(VERSION 3.23)
project(fucsCar10d)

set(CMAKE_CXX_STANDARD 17)

include_directories(vendor)
add_executable(fucsCar10d
        src/main.cpp
        src/opengl_utils.cpp
        vendor/stb_image.cpp
        vendor/stb_image.h)

target_link_libraries(fucsCar10d -lglfw -lGLEW -lGL)
