#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <memory>
#include "Scene.h"
#include "Model.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <set>
#include <chrono>

#include "window.h"
#include "VulkanTypes.h"

// Uniform bufferin rakenne shader-viestintää varten
struct UniformBufferObject {
    glm::mat4 transform;
};

class VulkanRenderer {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    VulkanRenderer(window& appWindow, uint32_t deviceIndex);
    ~VulkanRenderer();

    // Estetään kopiointi resurssien hallinnan vuoksi
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;

    void drawFrame(const Scene& scene);
    void waitIdle() { vkDeviceWaitIdle(m_device); }
    VkDevice getDevice() { return m_device; }
    VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }
    VkCommandPool getCommandPool() { return m_commandPool; }
    VkQueue getGraphicsQueue() { return m_graphicsQueue; }
private:
    // --- Konfiguraatio ---
    window& m_window;
    uint32_t m_preferredDeviceIndex;
    uint32_t m_currentFrame = 0;

    // --- Vulkan Core ---
    VkInstance       m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR     m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice         m_device = VK_NULL_HANDLE;

    // --- Jonot (Queues) ---
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    // --- Swapchain & Renderöinti ---
    VkSwapchainKHR              m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage>        m_swapChainImages;
    VkFormat                    m_swapChainImageFormat;
    VkExtent2D                  m_swapChainExtent;
    std::vector<VkImageView>    m_swapChainImageViews;
    std::vector<VkFramebuffer>  m_swapChainFramebuffers;
    VkRenderPass                m_renderPass = VK_NULL_HANDLE;

    // --- Grafiikkaputki (Pipeline) & Resurssit ---
    VkPipelineLayout      m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline            m_graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool      m_descriptorPool = VK_NULL_HANDLE;

	std::unique_ptr<Model> m_squareModel;// Malli, joka sisältää geometriaa (esim. kolmioita) piirtämiseen ikkunaan

    std::vector<VkBuffer>        m_uniformBuffers;
    std::vector<VkDeviceMemory>  m_uniformBuffersMemory;
    std::vector<void*>           m_uniformBuffersMapped;
    std::vector<VkDescriptorSet> m_descriptorSets;

    // --- Komentopuskurit ---
    VkCommandPool             m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    // --- Synkronointi ---
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores; // per-swapchain-image semaphores
    std::vector<VkFence>     m_inFlightFences;
    std::vector<VkFence>     m_imagesInFlight; // Seuraa swapchain-kuvien käyttöastetta

    // --- Sisäiset metodit ---
    void initVulkan();
    void cleanup();
    void cleanupSwapChain();
    void recreateSwapChain();

    // Alustusmetodit
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderpass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();
    void createDescriptorSetLayout();
    void createUniformBuffer();
    void createDescriptorPool();
    void createDescriptorSets();

    // Apumetodit
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void updateUniformBuffer(uint32_t currentFrame);
    // VulkanRenderer.h
 // ...
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Scene& scene);

    void updateUniformBufferForModel(uint32_t currentFrame, const glm::mat4& matrix);

    // Swapchainin apumetodit
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR      chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR        chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D              chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    // Laitteen valinnan apumetodit
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};