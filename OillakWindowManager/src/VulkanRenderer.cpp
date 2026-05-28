#include "VulkanRenderer.h"
#include <stdexcept>
#include <vector>
#include <iostream>
#include "VulkanUtils.h"
#include <print>
#include <cassert>
#include <algorithm>
#include <limits>

VulkanRenderer::VulkanRenderer(window& appWindow, uint32_t deviceIndex) : m_window(appWindow), m_preferredDeviceIndex(deviceIndex) {
    initVulkan();
}

VulkanRenderer::~VulkanRenderer() {
    cleanup();
}

void VulkanRenderer::initVulkan() {
    createInstance();
    createSurface();
  
	pickPhysicalDevice(); //etsitään fyysinen laite, joka tukee Vulkania
	createLogicalDevice(); //TODO: luodaan looginen laite, joka kommunikoi fyysisen laitteen kanssa
    createSwapChain();
    createImageViews();
	createRenderpass();
    createFramebuffers();
    createGraphicsPipeline();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
    
}
void VulkanRenderer::drawFrame() {
    // 1. Odotetaan, että edellinen ruutu on piirretty valmiiksi (CPU odottaa)
    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    // Lukitaan aita uudelleen tätä framea varten
    vkResetFences(m_device, 1, &m_inFlightFence);

    // 2. Pyydetään Swapchainilta seuraava vapaa kuva
    uint32_t imageIndex;
    vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    // 3. Nollataan ja nauhoitetaan komentopuskuri uusiksi
    vkResetCommandBuffer(m_commandBuffer, 0);
    recordCommandBuffer(m_commandBuffer, imageIndex);

    // 4. Määritetään, miten komennot lähetetään näytönohjaimelle
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Odota, että kuva on oikeasti saatavilla ennen kuin aletaan kirjoittaa siihen värejä
    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // Mitä lähetetään
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    // Mitä semaforea signaaloidaan, kun piirtäminen on valmis
    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // 5. LÄHETETÄÄN KOMENNOT GPU:lle! (Samalla avataan aita, kun työ on valmis)
    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Komentopuskurin lähetys jonoon epäonnistui!");
    }

    // 6. Näytetään valmis kuva ruudulla
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores; // Odota, että renderöinti on valmis

    VkSwapchainKHR swapChains[] = { m_swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_presentQueue, &presentInfo);
}
void VulkanRenderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // TÄRKEÄÄ: Aita luodaan valmiiksi "avattuun" tilaan, jotta ensimmäinen frame 
    // ei jää ikuisesti odottamaan aiempaa, olematonta framea.
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Synkronointiobjektien luonti epäonnistui!");
    }
}
void VulkanRenderer::createInstance() {
    // 1. Sovelluksen tiedot
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Oma Graphics Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Oillak Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    // Moderni Vulkan-versio (1.3 on erittäin hyvä lähtökohta nykypäivänä)
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // 2. Instanssin luontitiedot
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // 3. Ikkunointijärjestelmän vaatimat laajennukset (Win32 API)
    std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME //Tämä lisätty tukemaan uudempaa swapchain-kyselyä, joka on hyödyllinen tulevissa vaiheissa
    };

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Validation layerit lisätään myöhemmin tutoriaalin edetessä
    createInfo.enabledLayerCount = 0;

    // 4. Luodaan itse instanssi ja tarkistetaan virheet
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Vulkan-instanssin luonti epäonnistui!");
    }
    std::cout << "Vulkan Instance luotu onnistuneesti!" << std::endl;
}

void VulkanRenderer::createSurface()
{
    std::printf("Luodaan Vulkan Surface...\n");

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = m_window.getHandle();          // Täältä tulee ikkunan kahva
    createInfo.hinstance = m_window.getHInstance();  // Täältä tulee ohjelman kahva

    // vkCreateWin32SurfaceKHR on funktio, joka yhdistää Vulkanin Win32-ikkunaan
    if (vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Ikkunan pinnan (Surface) luonti epäonnistui!");
    }

    std::printf("...Vulkan Surface luotu onnistuneesti!\n");
}

void VulkanRenderer::createSwapChain()
{
    std::printf("Luodaan Swapchain...\n");
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

    // Otetaan talteen valitut asetukset apufunktioiden avulla
    // Huom: Koska käytämme Capabilities2:sta, pinnan kyvyt ovat '.surfaceCapabilities' alaisuudessa
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities.surfaceCapabilities);

    // Määritellään montako kuvaa halutaan ketjuun (pyydetään minimi + 1, jotta saadaan puskuria)
    uint32_t imageCount = swapChainSupport.capabilities.surfaceCapabilities.minImageCount + 1;

    // Varmistetaan ettei ylitetä näytönohjaimen maksimia (0 tarkoittaa ettei ylärajaa ole)
    if (swapChainSupport.capabilities.surfaceCapabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.surfaceCapabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // 1, ellei tehdä stereoskooppista 3D-peliä
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Piirretään suoraan tähän kuvaan

    // Tarkistetaan, ovatko grafiikka- ja esitysjonot eri perheissä
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Jaettu tila, jos eri jonot
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Paras suorituskyky, jos sama jono hallitsee molempia
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Älä sekoita ikkunaa työpöydän taustaan
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // Suorituskykyparannus: älä piirrä pikseleitä jotka ovat toisen ikkunan takana
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Luodaan itse Swapchain!
    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Swapchainin luonti epäonnistui!");
    }

    // Haetaan luodut Swapchain-kuvat talteen muistiin
    chk(vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr));
    m_swapChainImages.resize(imageCount);
    chk(vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data()));

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;

    std::printf("...Swapchain luotu onnistuneesti! (%d kuvaa käytössä, resoluutio: %dx%d)\n", imageCount, extent.width, extent.height);
}

void VulkanRenderer::createImageViews() {
    std::printf("Luodaan Image Views Swapchain-kuville...\n");

    // Varataan vektorille tilaa saman verran kuin kuvia on
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Kohdellaan kuvaa tavallisena 2D-kuvana
        createInfo.format = m_swapChainImageFormat;  // Käytetään samaa formaattia kuin swapchainissa

        // Komponenttien swizzle (värikanavien reititys). IDENTITY = pidetään oletuksena (R->R, G->G jne.)
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // SubresourceRange määrittää kuvan käyttötarkoituksen ja laajuuden
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Kuva sisältää väritietoa
        createInfo.subresourceRange.baseMipLevel = 0;                      // Ei mipmappeja (taso 0)
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;                    // Vain 1 taso (ei esim. VR-tuplasilmäkuvia)
        createInfo.subresourceRange.layerCount = 1;

        // Luodaan Image View!
        if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Virhe: Image View'n luonti epäonnistui indeksille " + std::to_string(i));
        }
    }

    std::printf("...Kaikki (%zu kpl) Image View't luotu onnistuneesti!\n", m_swapChainImageViews.size());
}

SwapChainSupportDetails VulkanRenderer::querySwapChainSupport(VkPhysicalDevice device)
{

    SwapChainSupportDetails details{};

    // Kerrotaan mitä pintaa ollaan tutkimassa
    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo.surface = m_surface;

    // Alustetaan palautettava rakenne (Vulkan vaatii sType-tiedon!)
    details.capabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    details.capabilities.pNext = nullptr;

    // Kutsutaan uutta modernia funktiota
    chk(vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &surfaceInfo, &details.capabilities));

    // 2. Haetaan tuetut formaatit (Pikseliformaatit ja väriavaruudet)
    uint32_t formatCount = 0;
    chk(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr));

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        chk(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data()));
    }

    // 3. Haetaan tuetut esitystilat (Present modes, esim. V-Sync tai Mailbox)
    uint32_t presentModeCount = 0;
    chk(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr));

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        chk(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data()));
    }

    return details;
}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& format : availableFormats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)// Tämä on yleisesti suositeltu formaatti, joka tukee 8-bittisiä värejä ja sRGB-väriavaruutta
        {
			return format; // Löydettiin haluttu formaatti, palautetaan se
		}
	}
	return availableFormats[0]; // Jos haluttu formaatti ei löytynyt, palautetaan ensimmäinen saatavilla oleva formaatti (varmistetaan, että lista ei ole tyhjä ennen tätä!)
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& presentMode : availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            std::printf("Käytetään esitystilaa: MAILBOX (Triple-Buffering)\n");
            return presentMode;
        }
    }

    // TÄMÄ LISÄYS VARMISTAA FALLBACKIN:
    std::printf("Käytetään esitystilaa: FIFO (Standard V-Sync)\n");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
        return capabilities.currentExtent;
    }
    else {
        // Jos Windows antaa meille vapaat kädet, lukitaan koko moottorin alustuksen mukaiseksi (800x600)
        VkExtent2D actualExtent = { 800, 600 };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}
void VulkanRenderer::pickPhysicalDevice() {
    uint32_t deviceCount = { 0 };
    chk(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));

    std::vector<VkPhysicalDevice> devices(deviceCount);
    chk(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

    std::cout << "Huom, " << deviceCount << " olemassa olevaa laitetta, jotka tukevat Vulkania.\n" << std::endl;

    // 1. Kokeillaan ensin komentoriviltä annettua laitetta (jos indeksi on validi)
    if (m_preferredDeviceIndex < deviceCount && isDeviceSuitable(devices[m_preferredDeviceIndex])) {
        m_physicalDevice = devices[m_preferredDeviceIndex];
    }
    // 2. Jos se ei sopinut (tai ei annettu), etsitään listasta ensimmäinen sopiva
    else {
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                m_physicalDevice = device;
                break;
            }
        }
    }

    // 3. Varmistetaan, että jotain löytyi
    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Virhe: Sopivaa GPU:ta ei löytynyt!");
    }

    // 4. Tulostetaan LOPULTA VALITUN laitteen tiedot
    VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties);

    std::cout << "Valittu laite: " << deviceProperties.properties.deviceName << "\n";
    std::cout << "Sopiva laite valittu onnistuneesti!" << std::endl;
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) {
    std::printf("Tarkistetaan laitteen sopivuus.\n");
    if (device == VK_NULL_HANDLE) {
		return false; // Ei laitetta, ei sopiva
	}
	QueueFamilyIndices indices = findQueueFamilies(device);
    if (indices.isComplete()) {
        std::printf("Laite sopiva.\n");

		return true; // Laite sopii, kaikki tarvittavat perheindeksit löytyivät
    }
    std::printf("Laite ei sopiva, vajaat perheindeksit.\n");
	return false; // Tarvittavia perheindeksejä ei löytynyt, laite ei sopiva
}

QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device) {
	std::printf("Etsitään laitteen perheindeksit...");
    QueueFamilyIndices indices;

    // 1. Kysytään, kuinka monta jonoperhettä laitteella on
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    // 2. Haetaan itse jonoperheiden tiedot
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // 3. Käydään lista läpi ja etsitään tarvitsemamme jonot
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {

        // Löytyikö grafiikkajono?
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        // Löytyikö ikkunaan piirtämisen (present) tuki?
        // Koska rakennat Win32-ikkunaa, käytämme Vulkanin Win32-spesifiä funktiota
        VkBool32 presentSupport = vkGetPhysicalDeviceWin32PresentationSupportKHR(device, i);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        // Jos löysimme jo molemmat, voimme lopettaa etsimisen ja säästää aikaa!
        if (indices.isComplete()) {
            break;
        }

        i++;
    }
    std::printf("...Perhe indeksit löytyivät\n");
    return indices;
}

void VulkanRenderer::createLogicalDevice() {
	std::printf("Luodaan loogista laitetta... \n");
    // 1. Haetaan taas ne jonojen indeksit siltä laitteelta, jonka juuri valitsimme
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    // 2. Määritellään, mitä jonoja halutaan luoda.
    // std::set varmistaa, että jos graphicsFamily ja presentFamily ovat sama (esim. indeksi 0),
    // se luodaan vain kerran!
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f; // Prioriteetti 0.0 - 1.0 (vaaditaan aina)
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // 3. Määritellään laitteen ominaisuudet (tässä vaiheessa emme tarvitse vielä erikoisuuksia)
    VkPhysicalDeviceFeatures deviceFeatures{};

    // 4. Varsinaisen Loogisen Laitteen luontitiedot
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    // Laajennukset (Extensions).
	std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME // Swapchain-laajennus on ehdoton, jos haluamme piirtää ikkunaan!
	};
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()); // Laajennukset on pakko määritellä, muuten laite ei tue niitä, vaikka fyysinen laite tukisi!
	createInfo.ppEnabledExtensionNames = deviceExtensions.data(); // Validation layerit (Debug) lisätään myöhemmin tutoriaalin edetessä

    // 5. Luodaan Looginen Laite!
    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Loogisen laitteen (Logical Device) luonti epäonnistui!");
    }

    // 6. Haetaan jonojen "kahvat" (handles), jotta voimme myöhemmin lähettää niille komentoja
    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);

    std::cout << "...Looginen laite ja jonot luotu onnistuneesti!\n";
}

void VulkanRenderer::createRenderpass() {
    std::printf("Luodaan Render Pass...\n");
	// Render Pass määrittelee, miten renderöinti tapahtuu. Tässä vaiheessa määritellään vain yksinkertainen väribufferi ilman syvyysbufferia tai monimoninäytön tukea. Tämä on hyvä lähtökohta, ja myöhemmin tutoriaalissa voidaan laajentaa tätä monimutkaisempaan renderpassiin.
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapChainImageFormat; // Käytetään samaa formaattia kuin swapchainin kuvat
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Ei multisamplingia (antialiasing)
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Tyhjennetään kuva joka kerta ennen piirtämistä
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Tallennetaan piirrettävä kuva muistiin esitystä varten
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Emme käytä stenciliä, joten ei väliä
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Emme käytä stenciliä, joten ei väliä
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Alkuperäinen layout ei merkitse mitään, koska tyhjennämme sen joka kerta
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Lopullinen layout on esityskelpoinen, jotta voimme näyttää sen ikkunassa
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // Viittaa ensimmäiseen (ja ainoaan) liitteeseen renderpassissa
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout, jossa renderöinti tapahtuu
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Tämä on grafiikkasubpassi
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // Käytetään määritettyä väriliitettä
	//jotta vulkan tietää, että meidän renderöinti riippuu edellisestä renderöinnistä (esim. swapchainin kuva on valmis esitettäväksi), määritellään subpass-dependenssi
	// Tämä varmistaa, että renderöinti odottaa swapchainin kuvan olevan valmis ennen kuin yrittää piirtää siihen, ja että kuva on esityskelpoinen ennen kuin se näytetään ikkunassa
	//ilman tätä dependenssiä saatat kohdata ongelmia, kuten mustia ruutuja tai virheitä renderöinnissä, koska Vulkan ei tiedä, milloin swapchainin kuva on valmis käytettäväksi
    VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Riippuvuus ulkoisesta subpassista (esim. edellisestä renderöinnistä)
	dependency.dstSubpass = 0; // Riippuvuus ensimmäisestä subpassista (meidän ainoa subpassi)
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Odotetaan, että edellinen renderöinti on valmis ennen kuin aloitamme tämän 
	dependency.srcAccessMask = 0; // Ei tarvitse odottaa mitään erityistä pääsyä, koska edellinen renderöinti on valmis 
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Tämä subpassi käyttää väriliitettä, joten odotetaan, että se on valmis ennen kuin aloitamme tämän
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Tarvitsemme kirjoitusoikeuden väriliitteeseen, jotta voimme renderöidä siihen
	// Render Passin luontitiedot
    VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Virhe: Render Passin luonti epäonnistui!");
	}
	std::printf("...Render Pass luotu onnistuneesti!\n");

}
VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();

    // Vulkan vaatii, että koodiosoitin on tyyppiä uint32_t*, joten tehdas castataan char* -> uint32_t*
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Shader modulen luonti epäonnistui!");
    }

    return shaderModule;
}
#include <fstream>

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Virhe: Tiedoston avaaminen epäonnistui: " + filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}
void VulkanRenderer::createGraphicsPipeline() {
    
        std::printf("Aloitetaan grafiikkaputken (Graphics Pipeline) alustus...\n");

        // === 1. LADATAAN SHADER-BINÄÄRIT SISÄÄN ===
        auto vertShaderCode = readFile("vert.spv");
        auto fragShaderCode = readFile("frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        // Määritellään Vertex Shaderin vaihe putkelle
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main"; // Funktion nimi shaderissa

        // Määritellään Fragment Shaderin vaihe putkelle
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    // 2. Input Assembly - Vaihe (Miten pisteet yhdistetään geometriaksi)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Piirretään kolmioita
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 3. Viewport ja Scissor (Mille alueelle ikkunassa piirretään)
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapChainExtent.width;
    viewport.height = (float)m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // 4. Rasterizer - Rasterointivaihe (Muuttaa geometriapisteet pikseleiksi)
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Sallitaan piirtäminen ruudulle
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  // Täytetään kolmio (vaihtoehtona esim. VK_POLYGON_MODE_LINE)
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;    // Karsitaan kolmion takapuolet tehon säästämiseksi
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Kolmion pisteiden suunta myötäpäivään
    rasterizer.depthBiasEnable = VK_FALSE;

    // 5. Multisampling (Reunanpehmennys - pidetään pois päältä toistaiseksi)
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // 6. Color Blending - Värien sekoitus (Miten uusi pikseli sekoittuu vanhaan)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE; // Ei läpinäkyvyyttä vielä tässä vaiheessa

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // 7. Pipeline Layoutin luonti (Tähän määritellään myöhemmin Uniform-muuttujat/matriisit)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Pipeline Layoutin luonti epäonnistui!");
    }
    // === 2. LUODAAN ITSE GRAFIIKKAPUTKI ===
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages; // Kytketään meidän shader-vaiheet mukaan!

    // Kytketään aiemmin luodut kiinteät asetukset
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Ei syvyystestiä vielä
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Ei dynaamisia tiloja vielä

    pipelineInfo.layout = m_pipelineLayout; // Kytketään layout
    pipelineInfo.renderPass = m_renderPass; // Kytketään Render Pass, johon putkea käytetään
    pipelineInfo.subpass = 0;               // Subpassin indeksi

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Ei käytetä pohjaputkia optimointiin
    pipelineInfo.basePipelineIndex = -1;

    // Luodaan itse putki!
    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Grafiikkaputken (Graphics Pipeline) luonti epäonnistui!");
    }

    std::printf("...Grafiikkaputki luotu onnistuneesti!\n");

    // === 3. TUHOTAAN SHADER MODUULIT ===
    // Kun putki on luotu, näytönohjain on kääntänyt koodin omalle konekielelleen. 
    // Väliaikaisia VkShaderModule-kahvoja ei enää tarvita, joten ne tuhotaan heti funktion lopussa.
    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}
void VulkanRenderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice); // Tarvitset tämän apufunktion

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Sallitaan puskurin uudelleenkäyttö
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Command Poolin luonti epäonnistui!");
    }
}
void VulkanRenderer::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Command Bufferin luonti epäonnistui!");
    }
}
void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Komentopuskurin aloittaminen epäonnistui!");
    }

    // 1. Aloitetaan Render Pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex]; // Tarvitset tämän listan!
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChainExtent;

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} }; // Musta taustaväri
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // 2. Kytketään meidän Graphics Pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    // 3. Piirretään kolmio! (3 vertexiä, 1 instanssi)
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    // 4. Lopetetaan Render Pass
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Virhe: Komentopuskurin tallennus epäonnistui!");
    }
}
void VulkanRenderer::createFramebuffers() {
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            m_swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Virhe: Framebufferin luonti epäonnistui!");
        }
    }
    std::printf("...Framebufferit luotu onnistuneesti (%zu kpl)!\n", m_swapChainFramebuffers.size());
}
void VulkanRenderer::cleanup() {
    // Tuhoa grafiikkaputki ja sen layout
    if (m_graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
        m_graphicsPipeline = VK_NULL_HANDLE;
    }
	if (m_pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		m_pipelineLayout = VK_NULL_HANDLE;
	}
    //tärkeää tuhoa kaikki Vulkan-resurssit päinvastaisessa järjestyksessä kuin ne luotiin, muuten voi tapahtua outoja virheitä ja muistivuotoja!
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
    // Tuhoa Image View't 
    for (auto imageView : m_swapChainImageViews) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, imageView, nullptr);
        }
    }
    m_swapChainImageViews.clear();
	//tuhoa swapchain ja siihen liittyvät resurssit (Swapchain)
	if (m_swapChain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	}
    //tuhoa pinta (Surface)
	if (m_surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    }
  
    if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);
    }
    if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
    }
    if (m_inFlightFence != VK_NULL_HANDLE) {
        vkDestroyFence(m_device, m_inFlightFence, nullptr);
    }
    for (auto framebuffer : m_swapChainFramebuffers) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
	//tuhota looginen laite (Device) 
	if (m_device != VK_NULL_HANDLE) {
		vkDestroyDevice(m_device, nullptr);
	}
    // Tuhotaan instanssi hallitusti ohjelman sulkeutuessa
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
    }
    
	std::cout << "Vulkan-resurssit tuhottu onnistuneesti!" << std::endl;

}