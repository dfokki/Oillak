cmake_minimum_required(VERSION 3.20)
project(Oillak VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

# ========================================================
# SHADERIEN AUTOMAATTINEN KÄÄNTÄMINEN (ROOT/SHADERS/)
# ========================================================
find_program(GLSLC glslc REQUIRED)

# Määritellään hakemisto, johon shaderit luodaan
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

set(SHADER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/OillakWindowManager/src/shader.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/OillakWindowManager/src/shader.frag"
)

set(SPV_FILES "")
foreach(SHADER_FILE ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(OUTPUT_FILE "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")
    
    add_custom_command(
        OUTPUT "${OUTPUT_FILE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}"
        COMMAND ${GLSLC} "${SHADER_FILE}" -o "${OUTPUT_FILE}"
        DEPENDS "${SHADER_FILE}"
        COMMENT "Käännetään shader: ${SHADER_NAME} -> ${OUTPUT_FILE}"
    )
    list(APPEND SPV_FILES "${OUTPUT_FILE}")
endforeach()

add_custom_target(CompileShaders DEPENDS ${SPV_FILES})

# ========================================================
# PROJEKTIN MÄÄRITTELY
# ========================================================

set(OWM_SOURCES
    OillakWindowManager/src/main.cpp
    OillakWindowManager/src/VulkanRenderer.cpp
    OillakWindowManager/src/window.cpp
    OillakWindowManager/src/Model.cpp
    OillakWindowManager/src/OillakEngine.cpp
    OillakWindowManager/src/Scene.cpp
)

add_executable(OillakWindowManager ${OWM_SOURCES})

# Varmistetaan että shaderit on käännetty ennen kuin OillakWindowManager kääntyy
add_dependencies(OillakWindowManager CompileShaders)

target_compile_definitions(OillakWindowManager PRIVATE UNICODE _UNICODE)
target_include_directories(OillakWindowManager PRIVATE OillakWindowManager/headers)
target_link_libraries(OillakWindowManager PRIVATE Vulkan::Vulkan)