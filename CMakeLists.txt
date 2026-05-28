cmake_minimum_required(VERSION 3.20)
project(Oillak VERSION 1.0 LANGUAGES CXX)

# Käytetään C++20 standardia (Vulkanin kanssa erittäin suositeltava)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Etsitään Vulkan SDK järjestelmästä (pakollinen)
find_package(Vulkan REQUIRED)

# 2. Määritellään lähdekooditiedostot
set(SOURCE_FILES
    OillakWindowManager/src/main.cpp
    OillakWindowManager/src/VulkanRenderer.cpp
    OillakWindowManager/src/window.cpp
)

# 3. Luodaan suoritettava ohjelma (Executable)
add_executable(OillakWindowManager ${SOURCE_FILES})
target_compile_definitions(OillakWindowManager PRIVATE UNICODE _UNICODE) # Windowsin UNICODE-määritelmät, jos käytetään Windowsia. Voit poistaa nämä, jos et käytä Windowsia tai et tarvitse UNICODE-tukea.
# 4. Kerrotaan mistä löytyy omat header-tiedostot (.h)
target_include_directories(OillakWindowManager PRIVATE 
    OillakWindowManager/headers
)

# 5. Linkitetään Vulkan-kirjastot ja lisätään sen vaatimat includet automaattisesti
target_link_libraries(OillakWindowManager PRIVATE Vulkan::Vulkan)

# =========================================================================
# 2. SANDBOX 
# =========================================================================
set(SANDBOX_SOURCES
    sandbox/main.cpp
)
# Luodaan oma suoritettava ohjelma nimellä "Sandbox"
add_executable(Sandbox ${SANDBOX_SOURCES})

# Jos haluat käyttää Vulkania myös sandboxin puolella testeissä:
target_link_libraries(Sandbox PRIVATE Vulkan::Vulkan)