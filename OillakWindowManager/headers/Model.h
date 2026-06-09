#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanTypes.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Model {
public: 
	/*Model(VkDevice device, 
		VkPhysicalDevice physicalDevice, 
		VkCommandPool commandPool,
		VkQueue graphicsQueue, 
		const std::vector<Vertex>& vertices,
		const std::vector<uint16_t>& indices,
		glm::vec3 position);*/
	Model(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
		VkQueue graphicsQueue, const std::vector<Vertex>& vertices,
		const std::vector<uint16_t>& indices, glm::vec3 position);

	~Model();
	//estetään kopiointi resurssien hallinnan vuoksi, koska se hallitsee GPU-resursseja, jotka eivät kestä tuplaamista
	Model(const Model&) = delete; //tässä käytetään delete -määrettä, joka estää kopiointikonstruktoria olemasta käytettävissä, mikä tarkoittaa, että et voi luoda uutta Model-objektia kopioimalla olemassa olevaa. Tämä on tärkeää, koska Vulkan-resurssien tuplatuhoamisen estämiseksi haluamme varmistaa, että Model-objekteja ei kopioida vahingossa.
	Model& operator=(const Model&) = delete; //tämä eroaa yllä olevasta copy constructorista, koska se estää copy assignment operatorin olemasta käytettävissä. Copy assignment operator on funktio, joka määrittelee, miten objekti käyttäytyy, kun sille annetaan toinen objekti samanaikaisesti luotaessa (esim. Model a = b;). Tämä on tärkeää, koska Vulkan-resurssien tuplatuhoamisen estämiseksi haluamme varmistaa, että Model-objekteja ei kopioida vahingossa.

	//sitoo mallin komentopuskuriin, jotta se voidaan piirtää ikkunaan
	void bind(VkCommandBuffer commandBuffer);
	//piirtää mallin ikkunaan, edellyttää, että bind on kutsuttu ensin
	void draw(VkCommandBuffer commandBuffer);	
	void update(float time); // Päivittää matriisin
	const glm::mat4& getModelMatrix() const { return m_modelMatrix; }
private:
	glm::mat4 m_modelMatrix = glm::mat4(1.0f);
	glm::vec3 m_position;
	VkDevice m_device;

	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer m_indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

	uint32_t m_indexCount = 0;

	// Apufunktio, joka luo vertex bufferin ja täyttää sen vertex-tiedoilla
	void createBuffer(VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void copyBuffer(VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
};
