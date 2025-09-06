#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

namespace AetherGUI {

const COLORREF COLOR_BG = RGB(18, 18, 18);
const COLORREF COLOR_TEXT = RGB(220, 220, 220);

HWND g_hMainWindow = nullptr;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            CreateWindowExW(0, L"BUTTON", L"Execute",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 10, 100, 30, hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr);
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH hBrush = CreateSolidBrush(COLOR_BG);
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

extern "C" {
    __declspec(dllexport) HWND CreateAetherGUI(HINSTANCE hInstance) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = L"AetherGUI";
        
        RegisterClassExW(&wc);
        
        g_hMainWindow = CreateWindowExW(0, L"AetherGUI", L"Aether",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            nullptr, nullptr, hInstance, nullptr);
        
        if (g_hMainWindow) {
            ShowWindow(g_hMainWindow, SW_SHOW);
            UpdateWindow(g_hMainWindow);
        }
        
        return g_hMainWindow;
    }
    
    __declspec(dllexport) void RunMessageLoop() {
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

}
