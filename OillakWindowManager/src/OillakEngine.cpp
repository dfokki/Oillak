#include "OillakEngine.h"

#include "VulkanRenderer.h"
OillakEngine::OillakEngine()
    : m_window(800, 600, L"Oillak") {
	SetConsoleOutputCP(CP_UTF8); // Asetetaan konsoli UTF-8 tilaan
    // Alustetaan renderer ikkunan avulla
    m_renderer = std::make_unique<VulkanRenderer>(m_window, 0);
}
OillakEngine::~OillakEngine() = default;

void OillakEngine::run() {
    float time = 0.0f;
    while (m_window.processMessages()) {
        time += 0.01f;

        // 1. Päivitetään mallien tila (ei piirretä vielä!)
        for (auto& model : m_currentScene.getModels()) {
            model->update(time);
        }

        // 2. Käsketään renderer piirtämään koko scene
        m_renderer->drawFrame(m_currentScene);
    }
}