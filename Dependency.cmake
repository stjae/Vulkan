include(ExternalProject)

set(DEP_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)
set(DEP_INCLUDE_DIR ${DEP_INSTALL_DIR}/include)
set(DEP_LIB_DIR ${DEP_INSTALL_DIR}/lib)

#spdlog
ExternalProject_Add(
        dep-spdlog
        GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
        GIT_TAG "v1.14.1"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
)

set(DEP_LIST ${DEP_LIST} dep-spdlog)

#glfw
ExternalProject_Add(
        dep-glfw
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        GIT_TAG "3.3.8"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_TESTS=OFF
        -DGLFW_BUILD_DOCS=OFF
)
set(DEP_LIST ${DEP_LIST} dep-glfw)

#glm
ExternalProject_Add(
        dep-glm
        GIT_REPOSITORY "https://github.com/g-truc/glm.git"
        GIT_TAG "0.9.9.8"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_BINARY_DIR}/dep-glm-prefix/src/dep-glm/glm
        ${DEP_INSTALL_DIR}/include/glm
)
set(DEP_LIST ${DEP_LIST} dep-glm)

#imgui
set(imgui_src
        ${PROJECT_SOURCE_DIR}/imgui/imgui_draw.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_tables.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_widgets.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_impl_glfw.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_impl_vulkan.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_stdlib.cpp)
if (WIN32)
    add_library(imgui ${imgui_src} ${PROJECT_SOURCE_DIR}/imgui/imgui_impl_win32.cpp)
else ()
    add_library(imgui ${imgui_src})
endif ()

target_include_directories(imgui PRIVATE ${DEP_INCLUDE_DIR})
add_dependencies(imgui dep-glfw)
set(DEP_LIBS ${DEP_LIBS} imgui)

# ImGuizmo
add_library(imguizmo ${PROJECT_SOURCE_DIR}/imgui/ImGuizmo.cpp)
add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)
target_include_directories(imguizmo PRIVATE ${PROJECT_SOURCE_DIR}/imgui)
set(DEP_LIBS ${DEP_LIBS} imguizmo)

# stb
ExternalProject_Add(
        dep-stb
        GIT_REPOSITORY "https://github.com/nothings/stb"
        GIT_TAG "master"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        TEST_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_BINARY_DIR}/dep-stb-prefix/src/dep-stb/stb_image.h
        ${DEP_INCLUDE_DIR}/stb/stb_image.h
)
set(DEP_LIST ${DEP_LIST} dep-stb)

# assimp
ExternalProject_Add(
        dep-assimp
        GIT_REPOSITORY "https://github.com/assimp/assimp.git"
        GIT_TAG "v5.3.1"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DASSIMP_WARNINGS_AS_ERRORS=OFF
)
set(DEP_LIST ${DEP_LIST} dep-assimp)

# Native File Dialog
ExternalProject_Add(
        dep-nativefiledialog
        GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git"
        GIT_TAG "v1.1.1"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DNFD_BUILD_TESTS=OFF
)
set(DEP_LIST ${DEP_LIST} dep-nativefiledialog)

# yaml
ExternalProject_Add(
        dep-yaml
        GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp.git"
        GIT_TAG "0.8.0"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DYAML_BUILD_SHARED_LIBS=ON
)
set(DEP_LIST ${DEP_LIST} dep-yaml)