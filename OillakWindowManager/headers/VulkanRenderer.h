#pragma once
#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include "window.h"
#include "VulkanTypes.h"
#include <set>

class VulkanRenderer {
public:
    // Konstruktori ottaa vastaan viitteen luomaasi ikkunaan
    VulkanRenderer(window& appWindow, uint32_t deviceIndex);
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

    // Uudet apufunktiot laitteen valintaan
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};