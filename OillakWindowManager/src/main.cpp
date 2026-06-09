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

      
            // 8 kulmaa 3D-avaruudessa
            std::vector<Vertex> vertices = {
                {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 0: Vasen-Ylä-Taka
                {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, // 1: Oikea-Ylä-Taka
                {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}}, // 2: Oikea-Ala-Taka
                {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}, // 3: Vasen-Ala-Taka
                {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}}, // 4: Vasen-Ylä-Etu
                {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}}, // 5: Oikea-Ylä-Etu
                {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}, // 6: Oikea-Ala-Etu
                {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}  // 7: Vasen-Ala-Etu
        };

        // 6 sivua, jokaisessa 2 kolmiota (yhteensä 36 indeksiä)
        std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0, // Taka
            4, 5, 6, 6, 7, 4, // Etu
            0, 4, 7, 7, 3, 0, // Vasen
            1, 5, 6, 6, 2, 1, // Oikea
            0, 1, 5, 5, 4, 0, // Ylä
            3, 2, 6, 6, 7, 3  // Ala
        };
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