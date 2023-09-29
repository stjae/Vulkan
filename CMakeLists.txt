cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME vulkan_testing)
set(CMAKE_CXX_STANDARD 17)
if (APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework IOKit -framework OpenGL")
endif()

project(${PROJECT_NAME})
add_executable(${PROJECT_NAME}
src/main.cpp
src/common.h
src/baseApp.cpp src/baseApp.h
src/baseAppInfo.h
src/vkUtil.cpp src/vkUtil.h
src/vkDebug.cpp src/vkDebug.h)

include(Dependency.cmake)
find_package(Vulkan REQUIRED)
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