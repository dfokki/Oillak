#pragma once
#include <string>
#include <fstream>
#include <chrono>
#include <sstream>

// Eteenpäinlauseke rajapinnalle
class ILoggable;

class OillakLogger {
public:
    // Otetaan vastaan kaksi polkua: suorituskyvylle (CSV) ja tapahtumille (TXT)
    OillakLogger(const std::string& perfLogPath, const std::string& eventLogPath);
    ~OillakLogger();

    // Estetään kopiointi
    OillakLogger(const OillakLogger&) = delete;
    OillakLogger& operator=(const OillakLogger&) = delete;

    // Päivitetyt lokitusfunktiot
    void logFrame(const ILoggable* targetObject);

   
    void logMessage(const std::string& category, const std::string& message);

private:
    // Non-templated sink to perform the actual I/O. The template forwards
    // here to avoid accidental recursive overload resolution.
    void logMessageImpl(const std::string& category, const std::string& message);

public:
    //template funktio jotta ei tarvitse erikseen tehdä logMessage-funktiota eri datatyypeille, vaan C++ hoitaa sen automaattisesti, eli ei enään std::to_string-tyyppisiä rajoituksia, vaan voit logata suoraan mitä tahansa dataa, joka voidaan kirjoittaa std::ostreamiin (kuten int, float, std::string, yms.)
    template<typename... Args>
    void logMessage(const std::string& category, Args&&... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args)); // fold expression (C++17+)
        logMessageImpl(category, oss.str());
    }

private:
    std::ofstream m_logFile;   // Suorituskyky (.csv)
    std::ofstream m_eventFile; // Tapahtumat (.txt)

    std::chrono::steady_clock::time_point TimeNow;
    int m_frameCount = 0;
    float m_fpsTimer = 0.0f;
    float m_currentFps = 0.0f;
};