#pragma once
#include <optional>
#include <vector>
#include <array>           
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
	// graphicsFamily ja presentFamily ovat std::optional-tyyppisiä, koska emme vielä tiedä, löytyvätkö ne
	std::optional<uint32_t> graphicsFamily; // Tähän tallennetaan grafiikkakortin perheindeksi, joka tukee grafiikkaa
	std::optional<uint32_t> presentFamily; // Tähän tallennetaan grafiikkakortin perheindeksi, joka tukee esitystä (presentaatiota)

	// apufunktio tarkistamaan, että kaikki tarvittavat perheindeksit on löydetty
	bool isComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value(); // Tarkistetaan, että molemmat perheindeksit on asetettu (eli löytyneet)
	}
};

// TÄMÄ RAKENNE OLI PUUTTUVANAS TAI VAJAANA:
struct SwapChainSupportDetails {
	VkSurfaceCapabilities2KHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
    float position[3]; // X, Y, z (2 floatia)
    float color[3];    // R, G, B (3 floatia)

    // Kerrotaan Vulkanille, kuinka paljon dataa on yhdessä pisteessä
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    // Kerrotaan Vulkanille, miten position ja color kaivetaan irti Vertex-rakenteesta
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        // 1. Sijainti (location = 0)
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // 3 floatia
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        // 2. Väri (location = 1)
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // 3 floatia
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};