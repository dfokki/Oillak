#include "window.h"
#include <stdexcept>

window::window(int width, int height, const std::wstring& title) : m_width(width), m_height(height) {
    hInstance = GetModuleHandle(nullptr);

    WNDCLASS wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        throw std::runtime_error("RegisterClass failed");
    }

    // TÄRKEÄÄ: Välitetään 'this' CreateWindowEx-kutsun viimeisenä parametrina
    hWnd = CreateWindowEx(
        0, wc.lpszClassName, title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, this
    );

    if (!hWnd) {
        throw std::runtime_error("CreateWindowEx failed");
    }
}

window::~window() {
    if (hWnd) DestroyWindow(hWnd);
    UnregisterClass(L"MyWindowClass", hInstance);
}

bool window::processMessages() {
    MSG msg{};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

HWND window::getHandle() const { return hWnd; }
HINSTANCE window::getHInstance() const { return hInstance; }

// --- UUDET FUNKTIOT ---
void window::getFramebufferSize(int& width, int& height) const {
    width = m_width;
    height = m_height;
}

void window::waitEvents() {
    MSG msg;
    // GetMessage odottaa (pysäyttää ohjelman), kunnes käyttöjärjestelmä antaa viestin
    if (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// --- PÄIVITETTY WNDPROC ---
LRESULT CALLBACK window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Kaivetaan ikkuna-olio esiin Windowsin syövereistä
    window* win = reinterpret_cast<window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (msg) {
    case WM_NCCREATE: {
        CREATESTRUCT* create = reinterpret_cast<CREATESTRUCT*>(lParam);
        win = reinterpret_cast<window*>(create->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)win);
        break;
    }
    case WM_SIZE:
        // Kun koko muuttuu, päivitetään olion tiedot
        if (win) {
            win->m_width = LOWORD(lParam);
            win->m_height = HIWORD(lParam);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}