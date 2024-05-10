set(GLSL_VALIDATOR "${Vulkan_INCLUDE_DIRS}/../bin/glslc")

file(GLOB GLSL_SOURCE_FILES
        "shader/*.frag"
        "shader/*.vert"
)

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shader/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shader/"
            COMMAND ${GLSL_VALIDATOR} ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
            DEPFILE "shader/common.glsl")
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
        shader
        DEPENDS ${SPIRV_BINARY_FILES}
)