cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME vulkanApp)
set(CMAKE_CXX_STANDARD 17)
if (APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework IOKit -framework OpenGL")
endif()
get_cmake_property(is_multi_config GENERATOR_IS_MULTI_CONFIG)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIR})

project(${PROJECT_NAME})
add_executable(${PROJECT_NAME}
src/main.cpp
src/common.h            src/common.cpp
src/baseApp.h           src/baseApp.cpp
src/engine.h            src/engine.cpp

src/scene.h             src/scene.cpp
src/meshData.h          src/meshData.cpp
src/mesh.h              src/mesh.cpp
src/camera.h            src/camera.cpp
src/light.h             src/light.cpp

# API
src/API/window.h        src/API/window.cpp
src/API/instance.h      src/API/instance.cpp
src/API/logger.h        src/API/logger.cpp
src/API/device.h        src/API/device.cpp
src/API/swapchain.h     src/API/swapchain.cpp
src/API/swapchainData.h
src/API/shader.h        src/API/shader.cpp
src/API/descriptor.h    src/API/descriptor.cpp
src/API/pipeline.h      src/API/pipeline.cpp
src/API/commands.h      src/API/commands.cpp
src/API/sync.h          
src/API/buffer.h        src/API/buffer.cpp        
)

include(Dependency.cmake)
include(Shader.cmake)

target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS} Vulkan::Vulkan)

add_dependencies(${PROJECT_NAME} ${DEP_LIST} Shaders)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_BINARY_DIR}/shaders"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
)
