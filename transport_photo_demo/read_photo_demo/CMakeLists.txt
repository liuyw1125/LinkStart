cmake_minimum_required(VERSION 3.19)
project(read_photo_demo)

set(CMAKE_CXX_STANDARD 14)

add_executable(read_photo_demo main.cpp)

target_link_libraries(read_photo_demo -lrt)