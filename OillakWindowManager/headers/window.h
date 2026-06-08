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
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
   
private:
    int m_width;
	int m_height;
    HWND hWnd;
    HINSTANCE hInstance;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
