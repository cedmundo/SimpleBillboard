function(add_shader_target TARGET_NAME FILE_PREFIX)
    set(SHADER_VERT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PREFIX}.vert.slang")
    set(SHADER_FRAG_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PREFIX}.frag.slang")

    set(SHADER_VERT_BIN "${CMAKE_CURRENT_BINARY_DIR}/${FILE_PREFIX}.vert.spv")
    set(SHADER_FRAG_BIN "${CMAKE_CURRENT_BINARY_DIR}/${FILE_PREFIX}.frag.spv")

    set(SHADER_VERT_RFL "${CMAKE_CURRENT_BINARY_DIR}/${FILE_PREFIX}.vert.json")
    set(SHADER_FRAG_RFL "${CMAKE_CURRENT_BINARY_DIR}/${FILE_PREFIX}.frag.json")

    add_custom_command(
            OUTPUT ${SHADER_VERT_BIN}
            COMMAND slangc ${SHADER_VERT_SRC}
              -profile spirv_1_0
              -target spirv
              -o "${SHADER_VERT_BIN}"
              -entry vertexMain
              -emit-spirv-via-glsl
              -reflection-json ${SHADER_VERT_RFL}
              -capability GLSL_150
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${SHADER_VERT_SRC}
            COMMENT "Compiling vertex shader"
    )

    add_custom_command(
            OUTPUT ${SHADER_FRAG_BIN}
            COMMAND slangc ${SHADER_FRAG_SRC}
              -profile spirv_1_0
              -target spirv
              -o "${SHADER_FRAG_BIN}"
              -entry pixelMain
              -emit-spirv-via-glsl
              -reflection-json ${SHADER_FRAG_RFL}
              -capability GLSL_150
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${SHADER_FRAG_SRC}
            COMMENT "Compiling fragment shader"
    )

    add_custom_target(${TARGET_NAME}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${SHADER_VERT_BIN} ${SHADER_FRAG_BIN}
            COMMENT "Slang Shaders"
            VERBATIM
    )
endfunction()

add_subdirectory(shaders)
