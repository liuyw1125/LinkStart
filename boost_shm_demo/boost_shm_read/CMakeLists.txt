cmake_minimum_required(VERSION 3.19)
project(boost_shm)

set(CMAKE_CXX_STANDARD 14)
set(BIN_NAME "sva.shm_read")


set(THIRDPARTY_BOOST_INCLUDE_DIR 
    ${PROJECT_SOURCE_DIR}/thirdparty/boost/include 
    CACHE 
    FILEPATH
    "Path to thirdparty/boost"
)
set(LOG_INCLUDE_DIR
        ${PROJECT_SOURCE_DIR}/src/log/
        )
find_package(Threads REQUIRED)

if(WIN32 OR APPLE OR ANDROID OR QNXNTO)
    set(THIRDPARTY_BOOST_LINK_LIBS ${CMAKE_THREAD_LIBS_INIT})
else() # Posix
    set(THIRDPARTY_BOOST_LINK_LIBS ${CMAKE_THREAD_LIBS_INIT} rt pthread)
endif()

try_compile(IS_THIRDPARTY_BOOST_OK
        ${CMAKE_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/thirdparty/boost/test/ThirdpartyBoostCompile_test.cpp
         CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${THIRDPARTY_BOOST_INCLUDE_DIR}"
         CXX_STANDARD 11
         LINK_LIBRARIES ${THIRDPARTY_BOOST_LINK_LIBS}
         OUTPUT_VARIABLE OUT
    )

set(IS_THIRDPARTY_BOOST_SUPPORTED ${IS_THIRDPARTY_BOOST_OK})

if(NOT IS_THIRDPARTY_BOOST_OK)
    message(STATUS ${OUT} "\nCouldn't compile thirdparty/boost. SHM Transport feature will be disabled!!!\n")
else()
    message(STATUS "Thirdparty/boost compiled OK")
endif()

mark_as_advanced(THIRDPARTY_BOOST_INCLUDE_DIR)

include_directories(./src/)
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/log/*.cpp)
file(GLOB_RECURSE HDR_FILES ${PROJECT_SOURCE_DIR}/src/*.h)




add_executable(${BIN_NAME} ${SRC_FILES} ${HDR_FILES})
target_compile_definitions(${BIN_NAME} PRIVATE
        $<$<BOOL:${STDOUTERR_LOG_CONSUMER}>:STDOUTERR_LOG_CONSUMER> # Enable StdoutErrConsumer as default LogConsumer
        )

target_include_directories(${BIN_NAME} PRIVATE
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
        ${LOG_INCLUDE_DIR}
)


target_link_libraries( ${BIN_NAME}
    ${THIRDPARTY_BOOST_LINK_LIBS}
    )

