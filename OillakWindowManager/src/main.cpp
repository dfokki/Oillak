#include "window.h"
#include <iostream>
#include "OillakEngine.h"
#include "VulkanRenderer.h"
int main(int argc, char* argv[]) {
    OillakEngine engine;


    try {
        uint32_t preferredDevice = 0;
        if (argc > 1) {
            preferredDevice = std::stoi(argv[1]);
        }

        // 1. Määritellään mallin pisteet täällä, ei enää rendererissä!
        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // Vasen ylä
            {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, // Oikea ylä
            {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}, // Oikea ala
            {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}  // Vasen ala
        };
        std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

        // 2. Haetaan Vulkan-työkalut rendereriltä
        auto* renderer = engine.getRenderer();
        glm::vec3 pos = glm::vec3(-0.5f, 0.0f, 0.0f); glm::vec3 pos2 = glm::vec3(0.5f, 0.0f, 0.0f);
        // Luodaan ensimmäinen neliö vasemmalle (-0.5f)
        auto square = std::make_unique<Model>(
            renderer->getDevice(),
            renderer->getPhysicalDevice(),
            renderer->getCommandPool(),
            renderer->getGraphicsQueue(),
            vertices,
            indices,
           pos
        );

        // Luodaan toinen neliö oikealle (+0.5f)
        auto square2 = std::make_unique<Model>(
            renderer->getDevice(),
            renderer->getPhysicalDevice(),
            renderer->getCommandPool(), 
            renderer->getGraphicsQueue(),
            vertices, 
            indices,
            pos2
        );

        // 4. Lisätään malli automaattisesti alustettuun sceneen
        engine.getCurrentScene().addModel(std::move(square));
        engine.getCurrentScene().addModel(std::move(square2));
        // 5. Käynnistetään pelisilmukka
        engine.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Virhe: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}