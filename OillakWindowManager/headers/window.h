#pragma once
#include <windows.h>
#include <string>

class window {
public:
    window(int width, int height, const std::wstring& title);
    ~window();

    bool processMessages();   // palauttaa false jos WM_QUIT
    HWND getHandle() const;   // Vulkanin surfacea varten
    HINSTANCE getHInstance() const; // Vulkanin surfacea varten

    // --- F1-resizing 8.6.2026 ---
    void getFramebufferSize(int& width, int& height) const;
    void waitEvents(); // Pysäyttää säikeen nukkumaan, kunnes ikkuna palautetaan

private:
    HWND hWnd;
    HINSTANCE hInstance;
    int m_width;
    int m_height;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};