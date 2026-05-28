#pragma once
#include <optional>
#include <vector>          // <-- TÄRKEÄ: Tarvitaan std::vectoria varten
#include <vulkan/vulkan.h> // <-- TÄRKEÄ: Tarvitaan Vulkan-tyyppejä varten

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