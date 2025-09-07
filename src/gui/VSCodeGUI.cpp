#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>
#include <string>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

namespace AetherGUI {

const COLORREF COLOR_BG = RGB(30, 30, 30);
const COLORREF COLOR_TEXT = RGB(212, 212, 212);
const COLORREF COLOR_ACCENT = RGB(0, 122, 204);

HWND g_hMainWindow = nullptr;

void DrawTetrisLogo(HDC hdc, int x, int y) {
    HBRUSH hBrush = CreateSolidBrush(COLOR_ACCENT);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    Rectangle(hdc, x, y, x+8, y+8);
    Rectangle(hdc, x-8, y+8, x, y+16);
    Rectangle(hdc, x, y+8, x+8, y+16);
    Rectangle(hdc, x+8, y+8, x+16, y+16);
    
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            CreateWindowExW(0, L"BUTTON", L"Execute", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 50, 100, 30, hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr);
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH hBrush = CreateSolidBrush(COLOR_BG);
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);
            
            DrawTetrisLogo(hdc, 20, 10);
            
            SetTextColor(hdc, COLOR_TEXT);
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, 50, 15, L"Aether VSCode", 13);
            
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
        wc.lpszClassName = L"AetherVSCode";
        
        RegisterClassExW(&wc);
        
        g_hMainWindow = CreateWindowExW(0, L"AetherVSCode", L"Aether VSCode",
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
