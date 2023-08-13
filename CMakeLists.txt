cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME io_board)

include(common_libs/pico-sdk/pico_sdk_init.cmake)

project(${PROJECT_NAME} C CXX ASM)

pico_sdk_init()

include(common_libs/common.cmake)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} src/io_board.cpp)

target_link_libraries(${PROJECT_NAME} pico_stdlib ${COMMON_LIBS} hardware_pwm hardware_dma)

pico_add_extra_outputs(${PROJECT_NAME})
