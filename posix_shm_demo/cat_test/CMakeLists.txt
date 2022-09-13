cmake_minimum_required(VERSION 3.19)
project(cat_test)

set(CMAKE_CXX_STANDARD 14)
set(BIN_NAME "cat_test")

add_executable(${BIN_NAME} main.cpp)
#FIND_PACKAGE(Threads  REQUIRED)
target_link_libraries( ${BIN_NAME}
        #${CMAKE_THREAD_LIBS_INIT}
        rt pthread
)

