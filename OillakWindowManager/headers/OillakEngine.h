#pragma once
#include "window.h"
#include <memory>
#include "scene.h"


// 1. Poistettiin #include "VulkanRenderer.h"
// 2. Lisätään Forward Declaration:
class VulkanRenderer;

class OillakEngine {
public:
    OillakEngine();
    ~OillakEngine(); // 3. LISÄÄ TÄMÄ! Purkaja pitää määritellä eksplisiittisesti.

    void run();
    Scene& getCurrentScene() { return m_currentScene; }
    VulkanRenderer* getRenderer() { return m_renderer.get(); }
private:
    window m_window;
    std::unique_ptr<VulkanRenderer> m_renderer;
    Scene m_currentScene;
    
};