#include "OillakLogger.h"
#include "ILoggable.h" // Muista sisällyttää rajapinta täällä!
#include <iostream>
#include <filesystem>

OillakLogger::OillakLogger(const std::string& perfLogPath, const std::string& eventLogPath) {


    namespace fs = std::filesystem;
	// Varmistetaan, että logihakemistot ovat olemassa ennen tiedostojen avaamista
    fs::path perfPath(perfLogPath);
    fs::path eventPath(eventLogPath);
    fs::path perfDir = perfPath.parent_path();          // .../out/tools or similar
    fs::path eventDir = eventPath.parent_path();        // .../out/tools or similar

    // Create each unique non-empty parent directory once
    std::vector<fs::path> dirsToCreate;
    if (!perfDir.empty()) dirsToCreate.push_back(perfDir);
    if (!eventDir.empty() && eventDir != perfDir) dirsToCreate.push_back(eventDir);

    for (const auto& dir : dirsToCreate) {
        if (!fs::exists(dir)) {
            std::error_code ec;
            if (!fs::create_directories(dir, ec)) {
                std::cerr << "Failed to create log directory: " << dir << " (" << ec.message() << ")\n";
                // handle error (fallback, throw, or continue)
            }
        }
    }

    // 1. Avataan suorituskykylogi (CSV)
    m_logFile.open(perfLogPath, std::ios::out | std::ios::trunc);
    if (m_logFile.is_open()) {
        m_logFile << "DeltaTime(ms),FPS,CustomData\n";
    }
    else {
        std::cerr << "OillakLogger Error: Ei voitu avata tiedostoa " << perfLogPath << "!\n";
    }

    // 2. Avataan tapahtumalogi (TXT) 
	// Sama tarkistus ja hakemiston luonti tapahtumalogiinkin, jotta ei tule ongelmia myöhemmin logatessa
    m_eventFile.open(eventLogPath, std::ios::out | std::ios::trunc);
    if (!m_eventFile.is_open()) {
        std::cerr << "OillakLogger Error: Ei voitu avata tiedostoa " << eventLogPath << "!\n";
    }

    // Alustetaan alkuaika ensimmäistä framea varten
    TimeNow = std::chrono::high_resolution_clock::now();
}

OillakLogger::~OillakLogger() {
    if (m_logFile.is_open()) m_logFile.close();
    if (m_eventFile.is_open()) m_eventFile.close(); // Muistetaan sulkea myös tämä
}

void OillakLogger::logFrame(const ILoggable* targetObject) {
    // 1. Lasketaan DeltaTime millisekunteina
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTimeMs = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - TimeNow).count();
    TimeNow = currentTime;

    // 2. Lasketaan FPS (päivittyy sekunnin välein)
    m_fpsTimer += deltaTimeMs;
    m_frameCount++;
    if (m_fpsTimer >= 1000.0f) {
        m_currentFps = static_cast<float>(m_frameCount);
        m_frameCount = 0;
        m_fpsTimer = 0.0f;
    }

    // 3. Kirjoitetaan tiedostoon
    if (m_logFile.is_open()) {
        std::string customData = "N/A";

        // KORJAUS: Ei tyyppitarkistuksia! C++ kutsuu automaattisesti oikeaa funktiota.
        if (targetObject) {
            customData = targetObject->getLogData();
        }

        m_logFile << deltaTimeMs << "," << static_cast<int>(m_currentFps) << "," << customData << "\n";
    }
}

void OillakLogger::logMessage(const std::string& category, const std::string& message) {
    // Forward to the implementation to keep a single sink.
    logMessageImpl(category, message);
}

void OillakLogger::logMessageImpl(const std::string& category, const std::string& message) {
    if (m_eventFile.is_open()) {
        // Tähän voi myöhemmin lisätä halutessaan kellonajan. Nyt se kirjoittaa siististi kansion ja viestin.
        m_eventFile << "[" << category << "]: " << message << "\n";

        // Tulostetaan samalla Visual Studion Output/Konsoli-ikkunaan kehitysmukavuuden vuoksi
        std::cout << "[" << category << "]: " << message << "\n";
    }
}