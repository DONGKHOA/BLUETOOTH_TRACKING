# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/aws/backoffAlgorithm
						 ${CMAKE_CURRENT_LIST_DIR}/aws/coreMQTT
						 ${CMAKE_CURRENT_LIST_DIR}/aws/common/posix_compat
						 ${CMAKE_CURRENT_LIST_DIR}/Bsp
						 ${CMAKE_CURRENT_LIST_DIR}/Middleware
						 ${CMAKE_CURRENT_LIST_DIR}/App
	)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(AWS_IOT)

target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "certs/root_cert_auth.crt" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "certs/client.crt" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "certs/client.key" TEXT)