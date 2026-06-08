#include "OillakEngine.h"
#include <sstream>

// ALUSTUSLISTA: Muuttujat alustetaan tässä järjestyksessä
OillakEngine::OillakEngine(int width, int height, const std::wstring& title, uint32_t deviceIndex)
    : m_logger("tools/performancelog.csv", "tools/eventlog.txt"), // Jos teit kahden logon järjestelmän (perf + event)
    m_window(width, height, title),
    m_renderer(m_window, deviceIndex, &m_logger) // Viedään loggerin osoite (&) renderöijälle
{
    // Konstruktorin sisäpuoli voi jäädä tyhjäksi, koska kaikki alustettiin jo ylhäällä!
}

OillakEngine::~OillakEngine() {
    m_renderer.waitIdle();
}

void OillakEngine::run() {
    // Päälooppi
    while (m_window.processMessages()) {
        m_renderer.drawFrame();

        // Koska m_logger on nyt luokan jäsenmuuttuja, kutsutaan sitä m_-etuliitteellä.
        // Koska 'this' on osoitin tähän engine-olioon itseensä, logFrame saa haluamansa ILoggable-osoittimen.
        m_logger.logFrame(this);
    }
}

// TÄÄLLÄ KOOSTETAAN DATA
std::string OillakEngine::getLogData() const {
    std::stringstream ss;

    // Esimerkki: haetaan rendereriltä kolmiomäärä (varmista että VulkanRendereristä löytyy tämä funktio)
    ss << m_renderer.getTriangleCount();

    return ss.str();
}