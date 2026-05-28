#include "window.h"
#include <iostream>
#include "VulkanRenderer.h"
int main(int argc, char* argv[]) {
	SetConsoleOutputCP(CP_UTF8); // Asetetaan konsoli UTF-8 tilaan, jotta erikoismerkit n‰kyv‰t oikein
    try {
       
        uint32_t preferredDevice = 0;
        if (argc > 1) {
            preferredDevice = std::stoi(argv[1]);
        }

        window window(800, 600, L"Oillak");
        VulkanRenderer renderer(window,preferredDevice);
		renderer.drawFrame(); // Piirret‰‰n ensimm‰inen ruutu, jotta n‰emme heti jotain ikkunassa
        while (window.processMessages()) {
			
			
            // T‰ss‰ voisi piirt‰‰ tai p‰ivitt‰‰ logiikkaa
        }
		renderer.waitIdle(); // Varmistetaan, ett‰ kaikki GPU-tyˆt on tehty ennen kuin tuhotaan resurssit
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
    }

    return 0;
}
