#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <wininet.h>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "wininet.lib")

namespace AetherGUI {

const COLORREF COLOR_BLACK = RGB(0, 0, 0);
const COLORREF COLOR_DARK_GREY = RGB(20, 20, 20);
const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_BLUE = RGB(86, 156, 214);

HWND g_hMainWindow = nullptr;

void DrawTetrisLogo(HDC hdc, int x, int y) {
    HBRUSH hBrush = CreateSolidBrush(COLOR_BLUE);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    Rectangle(hdc, x, y, x+6, y+6);
    Rectangle(hdc, x-6, y+6, x, y+12);
    Rectangle(hdc, x, y+6, x+6, y+12);
    Rectangle(hdc, x+6, y+6, x+12, y+12);
    
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

LRESULT CALLBACK VSCodeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Simple GUI elements
            CreateWindowExW(0, L"BUTTON", L"Execute", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 50, 100, 30, hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr);
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH hBrush = CreateSolidBrush(COLOR_BLACK);
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);
            
            DrawTetrisLogo(hdc, 10, 5);
            
            SetTextColor(hdc, COLOR_WHITE);
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, 30, 8, L"Aether ScriptBlox", 17);
            
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
        wc.lpfnWndProc = VSCodeWindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(COLOR_BLACK);
        wc.lpszClassName = L"AetherVSCode";
        
        RegisterClassExW(&wc);
        
        g_hMainWindow = CreateWindowExW(0, L"AetherVSCode", L"Aether",
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
// Force workflow trigger
