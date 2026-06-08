#pragma once
#include "window.h"
#include "VulkanRenderer.h"
#include "tools/OillakLogger.h" // Otetaan loggeri käyttöön
#include "tools/ILoggable.h"

class OillakEngine : public ILoggable {
public:
    OillakEngine(int width, int height, const std::wstring& title, uint32_t deviceIndex);
    ~OillakEngine();

    void run();
    std::string getLogData() const override;

private:
    // KRIITTINEN JÄRJESTYS: 
    // C++ alustaa muuttujat tässä järjestyksessä ylhäältä alaspäin!
    OillakLogger   m_logger;    // 1. Loggeri luodaan ensin, jotta se on valmis
    window         m_window;    // 2. Ikkuna luodaan seuraavaksi
    VulkanRenderer m_renderer;  // 3. Renderer luodaan viimeisenä, ja se saa &m_loggerin
};