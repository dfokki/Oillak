#include "window.h"
#include <iostream>
#include "VulkanRenderer.h"
int main(int argc, char* argv[]) {
	SetConsoleOutputCP(CP_UTF8); // Asetetaan konsoli UTF-8 tilaan, jotta erikoismerkit näkyvät oikein
    try {

        uint32_t preferredDevice = 0;
        if (argc > 1) {
            preferredDevice = std::stoi(argv[1]);
        }

        window window(800, 600, L"Oillak");
        VulkanRenderer renderer(window, preferredDevice);
        // Pääsilmukka, joka jatkuu, kunnes ikkuna suljetaan (WM_QUIT-viesti vastaanotetaan)
        while (window.processMessages()) {
            renderer.drawFrame(); //Tähän funktioon tallennetaan renderöintikomentoja, jotka suoritetaan joka ikiselle swapchainin kuvalle
       
        }
        renderer.waitIdle(); // Varmistetaan, että kaikki GPU-työt on tehty ennen kuin tuhotaan resurssit
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
    }

    return 0;
}
