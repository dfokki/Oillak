#include "window.h"
#include <stdexcept>

window::window(int width, int height, const std::wstring& title) {
    hInstance = GetModuleHandle(nullptr);

    WNDCLASS wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        throw std::runtime_error("RegisterClass failed");
    }

    hWnd = CreateWindowEx(
        0, wc.lpszClassName, title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, nullptr
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

HWND window::getHandle() const {
    return hWnd;
}

HINSTANCE window::getHInstance() const
{
    return hInstance;
}

LRESULT CALLBACK window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

