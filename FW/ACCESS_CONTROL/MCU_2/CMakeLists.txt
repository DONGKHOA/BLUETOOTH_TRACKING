# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS  ${CMAKE_CURRENT_LIST_DIR}/App 
                          ${CMAKE_CURRENT_LIST_DIR}/Middleware 
                          ${CMAKE_CURRENT_LIST_DIR}/Middleware/tbcmh
                          ${CMAKE_CURRENT_LIST_DIR}/../Env
                          ${CMAKE_CURRENT_LIST_DIR}/Device
                          ${CMAKE_CURRENT_LIST_DIR}/Bsp)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ACCESS_CONTROL_2)
