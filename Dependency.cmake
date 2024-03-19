include(ExternalProject)

set(DEP_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)
set(DEP_INCLUDE_DIR ${DEP_INSTALL_DIR}/include)
set(DEP_LIB_DIR ${DEP_INSTALL_DIR}/lib)

ExternalProject_Add(
        dep-spdlog
        GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
        GIT_TAG "v1.x"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
)

set(DEP_LIST ${DEP_LIST} dep-spdlog)
if (MSVC)
    set(DEP_LIBS ${DEP_LIBS} spdlogd)
else ()
    set(DEP_LIBS ${DEP_LIBS} spdlog)
endif ()

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
set(DEP_LIBS ${DEP_LIBS} glfw3)

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
set(imguiLIBS
        ${PROJECT_SOURCE_DIR}/imgui/imgui_draw.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_tables.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_widgets.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_impl_glfw.cpp
        ${PROJECT_SOURCE_DIR}/imgui/imgui_impl_vulkan.cpp)
if (WIN32)
    add_library(imgui ${imguiLIBS} ${PROJECT_SOURCE_DIR}/imgui/imgui_impl_win32.cpp)
else ()
    add_library(imgui ${imguiLIBS})
endif ()

target_include_directories(imgui PRIVATE ${DEP_INCLUDE_DIR})
add_dependencies(imgui dep-glfw)
set(DEP_LIBS ${DEP_LIBS} imgui)

# ImGuizmo
add_library(imguizmo ${PROJECT_SOURCE_DIR}/imgui/ImGuizmo.cpp)
add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)
target_include_directories(imguizmo PRIVATE ${PROJECT_SOURCE_DIR}/imgui)
set(DEP_LIBS ${DEP_LIBS} imguizmo)

# IconFontCppHeaders
ExternalProject_Add(
        dep-iconfontcppheaders
        GIT_REPOSITORY "https://github.com/juliettef/IconFontCppHeaders.git"
        GIT_TAG "main"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        TEST_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_BINARY_DIR}/dep-iconfontcppheaders-prefix/src/dep-iconfontcppheaders
        ${DEP_INSTALL_DIR}/include/font
)
set(DEP_LIST ${DEP_LIST} dep-iconfontcppheaders)

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

# tinyobjloader
ExternalProject_Add(
        dep-tinyobjloader
        GIT_REPOSITORY "https://github.com/tinyobjloader/tinyobjloader.git"
        GIT_TAG "v2.0-rc1"
        GIT_SHALLOW 1
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        TEST_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_BINARY_DIR}/dep-tinyobjloader-prefix/src/dep-tinyobjloader/tiny_obj_loader.h
        ${DEP_INCLUDE_DIR}/tiny_obj_loader.h
)
set(DEP_LIST ${DEP_LIST} dep-tinyobjloader)

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
set(DEP_LIBS ${DEP_LIBS} nfd)
