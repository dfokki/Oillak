#pragma once
#include <string>

class ILoggable {
public:
    // Virtuaalinen destruktori on pakollinen rajapinnoille
    virtual ~ILoggable() = default;

    // Jokaisen aliluokan täytyy toteuttaa tämä ja palauttaa haluamansa arvot tekstinä
    virtual std::string getLogData() const = 0;
};