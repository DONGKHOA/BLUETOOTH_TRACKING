idf_build_get_property(target IDF_TARGET)

# Set linker paths based on target
if(${IDF_TARGET} STREQUAL "esp32")
    set(links "-L ${CMAKE_CURRENT_SOURCE_DIR}/lib/esp32")
elseif(${IDF_TARGET} STREQUAL "esp32s2")
    set(links "-L ${CMAKE_CURRENT_SOURCE_DIR}/lib/esp32s2")
elseif(${IDF_TARGET} STREQUAL "esp32s3")
    set(links "-L ${CMAKE_CURRENT_SOURCE_DIR}/lib/esp32s3")
elseif(${IDF_TARGET} STREQUAL "esp32c3")    
    set(links "-L ${CMAKE_CURRENT_SOURCE_DIR}/lib/esp32c3")
endif()

# Declare component with dependencies
idf_component_register(
    INCLUDE_DIRS 
        include/
        include/tool
        include/typedef
        include/image
        include/math
        include/nn
        include/tvm
        include/layer
        include/detect
        include/model_zoo
    REQUIRES 
        esp_timer     # Previously added
        esp_partition # <-- Add this new dependency
)

# Link libraries
target_link_libraries(${COMPONENT_TARGET} INTERFACE ${links})

set(lib 
    libcat_face_detect.a
    libhuman_face_detect.a
    libcolor_detect.a
    libmfn.a
    libdl.a
)
target_link_libraries(${COMPONENT_TARGET} INTERFACE ${lib})