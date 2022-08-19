cmake_minimum_required(VERSION 3.19)
project(log_test)
set(BIN_NAME "LogMacrosAllActiveTests")
set(CMAKE_CXX_STANDARD 14)

set(LOG_COMMON_INCLUDE_DIR
        ${PROJECT_SOURCE_DIR}/log/
        )

# MACROS LOG TEST ALL
include_directories(./)
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/log/*.cpp)
file(GLOB_RECURSE HDR_FILES ${PROJECT_SOURCE_DIR}/src/*.h)


add_executable(${BIN_NAME} ${SRC_FILES})

target_compile_definitions(${BIN_NAME}  PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
target_include_directories(${BIN_NAME}  PRIVATE
        ${LOG_COMMON_INCLUDE_DIR} )

