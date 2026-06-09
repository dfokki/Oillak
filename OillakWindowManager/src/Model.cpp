#include "Model.h"
#include "VulkanUtils.h"
#include <cstring>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>

Model::Model(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
    VkQueue graphicsQueue, const std::vector<Vertex>& vertices,
    const std::vector<uint16_t>& indices, glm::vec3 position) // 7 parametria
    : m_device(device), m_indexCount(static_cast<uint32_t>(indices.size())), m_position(position) {
    // Alusta matriisi identiteettimatriisiksi (ei rotaatiota tai siirtoa alkuun)
    glm::mat4 translate = glm::mat4(1.0f);
    m_modelMatrix = glm::translate(translate, m_position);

    if (vertices.empty() || indices.empty()) {
        throw std::runtime_error("Model: vertices or indices vector is empty");
    }
    // ==========================================
    // 1. VERTEX BUFFERIN LUONTI (Käyttäen Staging-puskuria)
    // ==========================================
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(physicalDevice, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, vertexBufferSize, 0, &data);
    std::memcpy(data, vertices.data(), (size_t)vertexBufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(physicalDevice, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

    copyBuffer(commandPool, graphicsQueue, stagingBuffer, m_vertexBuffer, vertexBufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);

    // ==========================================
    // 2. INDEX BUFFERIN LUONTI
    // ==========================================
    VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
    VkBuffer indexStagingBuffer;
    VkDeviceMemory indexStagingBufferMemory;

    createBuffer(physicalDevice, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexStagingBuffer, indexStagingBufferMemory);

    vkMapMemory(m_device, indexStagingBufferMemory, 0, indexBufferSize, 0, &data);
    std::memcpy(data, indices.data(), (size_t)indexBufferSize);
    vkUnmapMemory(m_device, indexStagingBufferMemory);

    createBuffer(physicalDevice, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

    copyBuffer(commandPool, graphicsQueue, indexStagingBuffer, m_indexBuffer, indexBufferSize);

    vkDestroyBuffer(m_device, indexStagingBuffer, nullptr);
    vkFreeMemory(m_device, indexStagingBufferMemory, nullptr);
}

Model::~Model() {
    vkDeviceWaitIdle(m_device);
    if (m_indexBuffer != VK_NULL_HANDLE) vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    if (m_indexBufferMemory != VK_NULL_HANDLE) vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
    if (m_vertexBuffer != VK_NULL_HANDLE) vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    if (m_vertexBufferMemory != VK_NULL_HANDLE) vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
}

void Model::bind(VkCommandBuffer commandBuffer) {
    VkBuffer vertexBuffers[] = { m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
}

void Model::draw(VkCommandBuffer commandBuffer) { 
vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
}

// --- APUFUNKTIOT MUISTINHALLINTAAN ---

void Model::createBuffer(VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    chk(vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    chk(vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory));
    vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

void Model::copyBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(m_device, commandPool, 1, &commandBuffer);
}

uint32_t Model::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Sopivaa muistityyppia ei loytynyt puskurille!");
}
void Model::update(float time) {
    // 1. Luodaan siirtomatriisi (Translation)
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);

    // 2. Luodaan rotaatiomatriisi (Rotation), muutetaan aika radiaaneiksi
    float radians = glm::radians(time * 90.0f); // Pyörii 90 astetta sekunnissa
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0.0f, 0.0f, 1.0f));

    // 3. OIKEOA JÄRJESTYS: Translation * Rotation
    m_modelMatrix = translation * rotation;

}