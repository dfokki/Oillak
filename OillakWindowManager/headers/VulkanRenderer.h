#pragma once
#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include "window.h"
#include "VulkanTypes.h"
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include "tools/OillakLogger.h"
//Tätä matriisia käytetään vertex shaderissä, jotta voimme muuttaa objektien sijaintia, kokoa ja suuntaa ikkunassa.
struct UniformBufferObject {
	glm::mat4 transform; // Yleinen muunnosmatriisi, joka sisältää käännös-, kierto- ja skaalaustiedot.
   
};
class VulkanRenderer {
public:
	//ainakin loggeri tarvitsee tietää, montako kolmiota piirretään, jotta se voi logata sen suorituskykytiedostoon. Tässä tapauksessa meillä on vain yksi neliö, joka koostuu kahdesta kolmiosta, joten palautetaan 2.
    uint32_t getTriangleCount() const { return 2; }
    // Konstruktori ottaa vastaan viitteen luomaasi ikkunaan
    VulkanRenderer(window& appWindow, uint32_t deviceIndex, OillakLogger* logger);
    ~VulkanRenderer();

    // Estetään kopiointi (Vulkan-resurssien tuplatuhoamisen estämiseksi)
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
	void drawFrame(); //Tähän funktioon tallennetaan renderöintikomentoja, jotka suoritetaan joka ikiselle swapchainin kuvalle
	void waitIdle() {
		vkDeviceWaitIdle(m_device);
	}
private:
    window& m_window;
    uint32_t m_preferredDeviceIndex;
    OillakLogger* m_logger = nullptr;
    // Vulkanin ydinoliot
	// Vulkanin instanssi on kuin "Vulkanin pääkonttori". Se on ensimmäinen objekti, joka luodaan, ja se hallitsee kaikkea Vulkanin toimintaa.
    VkInstance m_instance = VK_NULL_HANDLE;
	// Vulkanin pinta (Surface) on ikkunaan liittyvä objekti, joka mahdollistaa piirtämisen ikkunaan. Se on kuin "ikkunan kangas", johon Vulkan voi piirtää.
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE; 
	// Vulkanin jono (Queue) on kuin "linja", johon lähetetään komentoja suoritettavaksi. Tarvitsemme yleensä vähintään yhden grafiikkajonon ja yhden esitysjono (present queue)
	VkQueue m_graphicsQueue = VK_NULL_HANDLE; 
	VkQueue m_presentQueue = VK_NULL_HANDLE; 
	// Valittu fyysinen laite (GPU)
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE; //Tänne tallennetaan valittu fyysinen laite, joka tukee Vulkania
	// Swapchain-resurssit
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	//Nämä liittyvät grafiikkaputkeen, joka määrittelee, miten renderöinti tapahtuu (shaders, fixed-function tilat, yms.)
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    //  
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
	// Vertex buffer ja sen muisti, joita käytetään geometriaan (esim. kolmioiden) piirtämiseen ikkunaan
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
	// Index buffer ja sen muisti, joita käytetään geometriaan (esim. kolmioiden) piirtämiseen ikkunaan, jos haluamme käyttää indeksoitua renderöintiä (index bufferin avulla voimme määritellä, miten vertexit yhdistetään kolmioiksi ilman, että meidän tarvitsee toistaa vertex-tietoja)
    VkBuffer m_indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
	// Descriptor setit ja uniform bufferit, joita käytetään shaderien datan syöttämiseen (esim. muunnosmatriisit)
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    VkBuffer m_uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_uniformBufferMemory = VK_NULL_HANDLE;
    // Sisäiset funktiot
    void initVulkan();
	void pickPhysicalDevice(); //Etsitään fyysinen laite, joka tukee Vulkania
	void createLogicalDevice(); //Luodaan looginen laite, joka kommunikoi fyysisen laitteen kanssa
    void createInstance();
	void createSurface(); //Luodaan Vulkanin surface ikkunalle, jotta voimme piirtää siihen
	void createSwapChain(); //Luodaan swapchain, joka hallitsee ikkunan kuvia ja niiden esitystä
	void createImageViews(); //Luodaan kuvaesitykset swapchainin kuville, jotta voimme käyttää niitä renderöinnissä
	void createRenderpass(); //Luodaan renderpass, joka määrittelee miten renderöinti tapahtuu (tähän liittyy mm. väribufferin ja syvyysbufferin määrittely)
	void createGraphicsPipeline(); //Luodaan grafiikkaputki, joka määrittelee, miten renderöinti tapahtuu (shaders, fixed-function tilat, yms.)
    VkShaderModule createShaderModule(const std::vector<char>& code);
	// Komentopuskuri ja komentobufferi, joita käytetään renderöintikomentoihin
    void createCommandPool();
    void createCommandBuffer();
	void createFramebuffers(); //Luodaan framebufferit swapchainin kuville, jotta voimme renderöidä niihin
    void createSyncObjects();
	// Vertex bufferin luonti ja hallinta, tarvitsemme tämän, jotta voimme piirtää geometriaa (esim. kolmioita) ikkunaan
    void createVertexBuffer();
	// Index bufferin luonti ja hallinta, tarvitsemme tämän, jos haluamme käyttää indeksoitua renderöintiä (index bufferin avulla voimme määritellä, miten vertexit yhdistetään kolmioiksi ilman, että meidän tarvitsee toistaa vertex-tietoja)
	void createIndexBuffer();
	// Descriptor setien ja uniform bufferin luonti ja hallinta, tarvitsemme nämä, jotta voimme syöttää dataa shaderille (esim. muunnosmatriisit)
    void createDescriptorSetLayout();
    void createUniformBuffer();
    void createDescriptorPool();
    void createDescriptorSets();
	// Uniform bufferin päivittäminen, jotta voimme muuttaa objektien sijaintia, kokoa ja suuntaa ikkunassa joka framella
    void updateUniformBuffer(); // Tätä kutsutaan joka framella
	// Renderöintikomentoja käsittelevät funktiot
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex); //Tähän funktioon tallennetaan renderöintikomennot, jotka suoritetaan joka ikiselle swapchainin kuvalle
   
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device); //Kysytään swapchainin tukemat ominaisuudet, formatit ja esitystavat

	//Swapchainin valintafunktiot, jotka valitsevat parhaan saatavilla olevan vaihtoehdon
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	// Komentopuskuri ja komentobufferi, joita käytetään renderöintikomentoihin
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE; // Tallennetaan yksi komentopuskuri
    // Synkronointiobjektit
    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence m_inFlightFence = VK_NULL_HANDLE;

    
   
    void cleanup();

    // Apufunktiot laitteen valintaan
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    // Apufunktio, jolla kysytään näytönohjaimelta oikeantyyppistä muistia
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

};