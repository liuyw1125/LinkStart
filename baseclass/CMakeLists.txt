cmake_minimum_required(VERSION 3.19)
project(baseclass)

set(CMAKE_CXX_STANDARD 14)

add_executable(baseclass main.cpp BaseShm.h BaseShm.cpp)