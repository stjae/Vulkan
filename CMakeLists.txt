cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME vulkanApp)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)

if (APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework IOKit -framework OpenGL -framework AppKit -framework UniformTypeIdentifiers -liconv")
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
        src/path.h
        src/main.cpp
        src/common.h src/common.cpp
        src/engine/app.h src/engine/app.cpp
        src/engine/engine.h src/engine/engine.cpp

        src/scene/scene.h src/scene/scene.cpp
        src/scene/meshBase.h src/scene/meshBase.cpp
        src/scene/mesh.h src/scene/mesh.cpp
        src/scene/camera.h src/scene/camera.cpp
        src/engine/ui.h src/engine/ui.cpp

        # vulkan
        src/window.h src/window.cpp
        src/vulkan/instance.h src/vulkan/instance.cpp
        src/vulkan/debugMessenger.h src/vulkan/debugMessenger.cpp
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
        src/scene/shadowCubemap.h
        src/scene/shadowCubemap.cpp
        src/vulkan/cubemap.cpp
        src/vulkan/cubemap.h
        src/scene/envCubemap.cpp
        src/scene/envCubemap.h
        src/pipeline/meshRender.cpp
        src/pipeline/meshRender.h
        src/pipeline/shadowCubemap.cpp
        src/pipeline/shadowCubemap.h
        src/pipeline/envCubemap.cpp
        src/pipeline/envCubemap.h
        src/pipeline/irradianceCubemap.cpp
        src/pipeline/irradianceCubemap.h
        src/pipeline/skyboxRender.cpp
        src/pipeline/skyboxRender.h
        src/pipeline/prefilteredCubemap.cpp
        src/pipeline/prefilteredCubemap.h
        src/scene/prefilteredCubemap.cpp
        src/scene/prefilteredCubemap.h
        src/pipeline/brdfLut.cpp
        src/pipeline/brdfLut.h
        src/engine/sceneSerializer.cpp
        src/engine/sceneSerializer.h
        src/pipeline/shadowMap.cpp
        src/pipeline/shadowMap.h
        src/scene/shadowMap.cpp
        src/scene/shadowMap.h
        src/scene/physics.cpp
        src/scene/physics.h
        src/pipeline/lineRender.cpp
        src/pipeline/lineRender.h
        src/enum.h
        src/scene/physicsDebugDrawer.h
        src/vulkan/sync.cpp
        src/scene/id.h
        src/scene/id.cpp
        src/engine/script/script.cpp
        src/engine/script/script.h
        src/engine/script/registry.cpp
        src/engine/script/registry.h
        src/time.h
        src/keycode.h
        src/scene/cascadedShadowMap.cpp
        src/scene/cascadedShadowMap.h
        src/scene/light.cpp
        src/scene/light.h
        src/pipeline/colorID.cpp
        src/pipeline/colorID.h
)

if (WIN32)
    set(BULLET_ROOT "C:/bullet3")
endif ()
find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${BULLET_LIBRARIES})

if (WIN32)
    set(MONO_INCLUDE_DIR "C:/Program Files/Mono/include/mono-2.0")
    set(MONO_LIB_DIR "C:/Program Files/Mono/lib")
    include_directories(${MONO_INCLUDE_DIR})
    target_link_directories(${PROJECT_NAME} PUBLIC ${MONO_LIB_DIR})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${MONO_LIB_DIR}/mono-2.0-sgen.lib)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/Mono/bin/mono-2.0-sgen.dll" ${PROJECT_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_directory "C:/Program Files/Mono/lib/mono/4.5" ${PROJECT_BINARY_DIR}/mono/lib/mono/4.5)
elseif (APPLE)
    set(MONO_INCLUDE_DIR "/opt/homebrew/include/mono-2.0")
    set(MONO_LIB_DIR "/opt/homebrew/lib")
    include_directories(${MONO_INCLUDE_DIR})
    target_link_directories(${PROJECT_NAME} PUBLIC ${MONO_LIB_DIR})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${MONO_LIB_DIR}/libmonosgen-2.0.a)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${MONO_LIB_DIR}/libmonosgen-2.0.dylib" ${PROJECT_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${MONO_LIB_DIR}/mono/4.5" ${PROJECT_BINARY_DIR}/mono/lib/mono/4.5)
endif ()

include(Dependency.cmake)
target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
file(GLOB LIBS "${DEP_INSTALL_DIR}/lib/*.lib" "${DEP_INSTALL_DIR}/lib/*.a" "${DEP_INSTALL_DIR}/lib/*.dylib")
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS} ${DEP_LIBS})
file(GLOB DLLS "${DEP_INSTALL_DIR}/bin/*.dll")
foreach (DLL ${DLLS})
    add_custom_command(TARGET ${PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E copy
            "${DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    )
endforeach ()

include(Shader.cmake)
add_dependencies(${PROJECT_NAME} ${DEP_LIST} shader)
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

configure_file(path.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/path.h)
