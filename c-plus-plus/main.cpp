#include "window.h"
#include <iostream>
int main(){

    try {
        window window(800, 600, L"Test Window Manager");

        while (window.processMessages()) {
            // T‰ss‰ voisi piirt‰‰ tai p‰ivitt‰‰ logiikkaa
        }
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
    }

    return 0;
}