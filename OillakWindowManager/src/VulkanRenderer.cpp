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

void VulkanRenderer::cleanup() {
	//tärkeää tuhoa kaikki Vulkan-resurssit päinvastaisessa järjestyksessä kuin ne luotiin, muuten voi tapahtua outoja virheitä ja muistivuotoja!
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