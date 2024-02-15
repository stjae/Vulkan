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
        src/baseApp.h src/baseApp.cpp
        src/engine.h src/engine.cpp

        src/scene/scene.h src/scene/scene.cpp
        src/scene/meshData.h src/scene/meshData.cpp
        src/scene/mesh.h src/scene/mesh.cpp
        src/scene/camera.h src/scene/camera.cpp
        src/imgui.h src/imgui.cpp

        # API
        src/API/window.h src/API/window.cpp
        src/API/device/instance.h src/API/device/instance.cpp
        src/API/device/logger.h src/API/device/logger.cpp
        src/API/device/device.h src/API/device/device.cpp
        src/API/device/queue.h src/API/device/queue.cpp
        src/API/swapchain.h src/API/swapchain.cpp
        src/API/shader.h src/API/shader.cpp
        src/API/descriptor.h src/API/descriptor.cpp
        src/API/pipeline.h src/API/pipeline.cpp
        src/API/command.h src/API/command.cpp
        src/API/sync.h
        src/API/buffer.h src/API/buffer.cpp
        src/API/memory.h src/API/memory.cpp
        src/API/image.h src/API/image.cpp
        src/viewport.h
        src/viewport.cpp
        src/scene/light.cpp
        src/scene/light.h
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