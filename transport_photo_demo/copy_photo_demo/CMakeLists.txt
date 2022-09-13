cmake_minimum_required(VERSION 3.19)
project(copy_photo_demo)

set(CMAKE_CXX_STANDARD 14)

add_executable(copy_photo_demo main.cpp)

target_link_libraries(copy_photo_demo -lrt)