#include "OillakEngine.h"
#include <iostream>

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    try {
        uint32_t preferredDevice = 0;
        if (argc > 1) {
            preferredDevice = std::stoi(argv[1]);
        }

        // Luodaan moottori/framework-olio ja k‰ynnistet‰‰n se
        OillakEngine engine(800, 600, L"Oillak Engine", preferredDevice);
        engine.run();
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Critical Error", MB_OK | MB_ICONERROR);
    }

    return 0;
}