cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME vulkanApp)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
if (APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework IOKit -framework OpenGL -framework AppKit -framework UniformTypeIdentifiers")
    find_package(Vulkan REQUIRED MoltenVK SPIRV-Tools)
    include_directories(${Vulkan_MoltenVK_INCLUDE_DIR})
    include_directories(${Vulkan_INCLUDE_DIR})
    link_directories(/usr/local/lib)
    link_libraries(
            Vulkan::Vulkan
            MoltenVK
    )
elseif (WIN32)
    find_package(Vulkan REQUIRED)
    link_libraries(Vulkan::Vulkan)
else ()
    message(unsupported OS)
endif ()

add_executable(${PROJECT_NAME}
        src/config.h
        src/main.cpp
        src/common.h src/common.cpp
        src/engine/baseApp.h src/engine/baseApp.cpp
        src/engine/engine.h src/engine/engine.cpp

        src/scene/scene.h src/scene/scene.cpp
        src/scene/meshData.h src/scene/meshData.cpp
        src/scene/mesh.h src/scene/mesh.cpp
        src/scene/camera.h src/scene/camera.cpp
        src/engine/ui.h src/engine/ui.cpp

        # vulkan
        src/window.h src/window.cpp
        src/vulkan/instance.h src/vulkan/instance.cpp
        src/vulkan/logger.h src/vulkan/logger.cpp
        src/vulkan/device.h src/vulkan/device.cpp
        src/vulkan/swapchain.h src/vulkan/swapchain.cpp
        src/vulkan/shader.h src/vulkan/shader.cpp
        src/vulkan/descriptor.h src/vulkan/descriptor.cpp
        src/vulkan/pipeline.h src/vulkan/pipeline.cpp
        src/vulkan/command.h src/vulkan/command.cpp
        src/vulkan/sync.h
        src/vulkan/buffer.h src/vulkan/buffer.cpp
        src/vulkan/memory.h src/vulkan/memory.cpp
        src/vulkan/image.h src/vulkan/image.cpp
        src/engine/viewport.h
        src/engine/viewport.cpp
        src/scene/light.cpp
        src/scene/light.h
        src/scene/shadowMap.h
        src/scene/shadowMap.cpp
)

include(Dependency.cmake)
include(Shader.cmake)

target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS})

add_dependencies(${PROJECT_NAME} ${DEP_LIST} Shaders)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
)

# Create config.h contains project directory
if (APPLE)
    set(PROJECT_DIR_OSX ${CMAKE_CURRENT_SOURCE_DIR})
elseif (WIN32)
    set(PROJECT_DIR_WINDOWS ${CMAKE_CURRENT_SOURCE_DIR})
endif ()

configure_file(config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)