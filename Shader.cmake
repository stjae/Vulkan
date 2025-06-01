set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslc")

file(GLOB GLSL_SOURCE_FILES
        "shader/*.frag"
        "shader/*.vert"
)

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${CMAKE_CURRENT_SOURCE_DIR}/shader/spv/${FILE_NAME}.spv")

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shader/spv/")

    execute_process(
            COMMAND ${GLSL_VALIDATOR} ${GLSL} -o ${SPIRV}
	    RESULT_VARIABLE result
            OUTPUT_VARIABLE output
            ERROR_VARIABLE error
    )

    if(NOT ${result} EQUAL 0)
        message(FATAL_ERROR "Failed to compile ${GLSL}:\n${error}")
    else()
        message(STATUS "Compiled ${GLSL} -> ${SPIRV}")
    endif()
endforeach (GLSL)