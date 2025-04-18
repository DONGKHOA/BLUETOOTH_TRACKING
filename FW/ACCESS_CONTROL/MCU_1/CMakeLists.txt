cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS  ${CMAKE_CURRENT_LIST_DIR}/App 
                          ${CMAKE_CURRENT_LIST_DIR}/Bsp 
                          ${CMAKE_CURRENT_LIST_DIR}/Device 
                          ${CMAKE_CURRENT_LIST_DIR}/Middleware
                          ${CMAKE_CURRENT_LIST_DIR}/../Env
                          ${CMAKE_CURRENT_LIST_DIR}/Middleware/lvgl
                          ${CMAKE_CURRENT_LIST_DIR}/Middleware/esp-dl) 

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(ACCESS_CONTROL_1)
