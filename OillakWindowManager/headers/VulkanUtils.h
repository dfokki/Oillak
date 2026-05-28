#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <string>

// Apufunktio muuttamaan VkResult tekstimuotoon
inline std::string resultToString(VkResult result) {
    switch (result) {
    case VK_SUCCESS: return "VK_SUCCESS";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        // ... voit lisätä tänne muita tarpeellisia
    default: return "UNKNOWN_ERROR (" + std::to_string(result) + ")";
    }
}

// Itse chk-funktio, joka ottaa mukaan tiedostonimen ja rivinumeron
inline void checkResult(VkResult result, const char* file, int line) {
    if (result != VK_SUCCESS) {
        std::string errorMsg = "Vulkan virhe: " + resultToString(result) +
            " tiedostossa " + file +
            " rivillä " + std::to_string(line);
        throw std::runtime_error(errorMsg);
    }
}

// Makro, joka "piilottaa" koodin sotkun
#define chk(result) checkResult(result, __FILE__, __LINE__)
