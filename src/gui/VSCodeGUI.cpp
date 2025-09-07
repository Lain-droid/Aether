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

const COLORREF COLOR_BLACK = RGB(0, 0, 0);
const COLORREF COLOR_DARK_GREY = RGB(20, 20, 20);
const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_BLUE = RGB(86, 156, 214);

HWND g_hMainWindow = nullptr;
HWND g_hInjectBtn = nullptr;
HWND g_hStatusText = nullptr;

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

// Injection function (calls backend)
bool PerformInjection() {
    // Load backend DLL
    HMODULE hBackend = LoadLibraryA("aether_backend.dll");
    if (!hBackend) return false;
    
    // Get injection function
    typedef bool (*InjectFunc)();
    InjectFunc inject = (InjectFunc)GetProcAddress(hBackend, "InjectIntoRoblox");
    if (!inject) {
        FreeLibrary(hBackend);
        return false;
    }
    
    bool result = inject();
    FreeLibrary(hBackend);
    return result;
}

LRESULT CALLBACK VSCodeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            g_hInjectBtn = CreateWindowExW(0, L"BUTTON", L"Inject",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                50, 100, 100, 40, hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr);
                
            g_hStatusText = CreateWindowExW(0, L"STATIC", L"Ready",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                50, 160, 300, 25, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
            break;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                SetWindowTextW(g_hStatusText, L"Injecting...");
                EnableWindow(g_hInjectBtn, FALSE);
                
                if (PerformInjection()) {
                    SetWindowTextW(g_hStatusText, L"Injection successful");
                } else {
                    SetWindowTextW(g_hStatusText, L"Injection failed");
                    EnableWindow(g_hInjectBtn, TRUE);
                }
            }
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH hBrush = CreateSolidBrush(COLOR_BLACK);
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);
            
            DrawTetrisLogo(hdc, 20, 20);
            
            SetTextColor(hdc, COLOR_WHITE);
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, 50, 25, L"Aether", 6);
            
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
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
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
