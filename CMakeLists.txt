cmake_minimum_required(VERSION 3.20)
project(Oillak VERSION 1.0 LANGUAGES CXX)

# Käytetään C++20 standardia (Vulkanin kanssa erittäin suositeltava)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Etsitään Vulkan SDK järjestelmästä (pakollinen)
find_package(Vulkan REQUIRED)

# =========================================================================
# 1. OILLAK WINDOW MANAGER (Pääohjelma)
# =========================================================================

# Kerätään lähdekoodit ja otsikot (Uudet tiedostot lisätty suoraan listoihin!)
set(OILLAK_SOURCES
    OillakWindowManager/src/main.cpp
    OillakWindowManager/src/OillakEngine.cpp
    OillakWindowManager/src/VulkanRenderer.cpp
    OillakWindowManager/src/window.cpp
    OillakWindowManager/tools/OillakLogger.cpp
)

set(OILLAK_HEADERS
    OillakWindowManager/headers/OillakEngine.h
    OillakWindowManager/headers/VulkanRenderer.h
    OillakWindowManager/headers/VulkanTypes.h
    OillakWindowManager/headers/VulkanUtils.h
    OillakWindowManager/headers/window.h
    OillakWindowManager/tools/OillakLogger.h
    OillakWindowManager/tools/ILoggable.h
)

# Luodaan suoritettava ohjelma
add_executable(OillakWindowManager ${OILLAK_SOURCES} ${OILLAK_HEADERS})

target_compile_definitions(OillakWindowManager PRIVATE UNICODE _UNICODE)

# Määritetään INCLUDE-polut
target_include_directories(OillakWindowManager PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/OillakWindowManager/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/OillakWindowManager
)

# Linkitetään Vulkan
target_link_libraries(OillakWindowManager PRIVATE Vulkan::Vulkan)

# =========================================================================
# 2. SANDBOX (Testausympäristö - Vain loggerin testaukseen)
# =========================================================================
set(SANDBOX_SOURCES
    sandbox/main.cpp
    OillakWindowManager/tools/OillakLogger.cpp 
)

# Sandbox tarvitsee vain loggerin ja rajapinnan, ei koko engineä!
add_executable(Sandbox 
    ${SANDBOX_SOURCES} 
    OillakWindowManager/tools/OillakLogger.h 
    OillakWindowManager/tools/ILoggable.h
)

# Samat include-polut sandboxille, jotta sekin löytää Oillakin työkalut
target_include_directories(Sandbox PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/OillakWindowManager/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/OillakWindowManager
)

target_link_libraries(Sandbox PRIVATE Vulkan::Vulkan)

# ============================================================================
# 3. AUTOMAATTINEN SHADER-KÄÄNNÖS (GLSLC)
# ============================================================================
find_program(VULKAN_GLSL_COMPILER NAMES glslc HINTS ${Vulkan_BIN_DIR})
if(VULKAN_GLSL_COMPILER)
    message(STATUS "Glslc-kääntäjä löydetty: ${VULKAN_GLSL_COMPILER}")
    
    # Vertex Shader
    add_custom_command(
        TARGET OillakWindowManager POST_BUILD
        COMMAND ${VULKAN_GLSL_COMPILER} "${CMAKE_CURRENT_SOURCE_DIR}/OillakWindowManager/src/shader.vert" -o "${CMAKE_CURRENT_BINARY_DIR}/vert.spv"
        COMMENT "Käännetään Vertex Shader SPIR-V muotoon..."
    )
    
    # Fragment Shader
    add_custom_command(
        TARGET OillakWindowManager POST_BUILD
        COMMAND ${VULKAN_GLSL_COMPILER} "${CMAKE_CURRENT_SOURCE_DIR}/OillakWindowManager/src/shader.frag" -o "${CMAKE_CURRENT_BINARY_DIR}/frag.spv"
        COMMENT "Käännetään Fragment Shader SPIR-V muotoon..."
    )
else()
    message(WARNING "glslc-kääntäjää ei löytynyt! Shaderit pitää kääntää käsin.")
endif()