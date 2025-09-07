#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <string>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "wininet.lib")

namespace AetherSetup {

const COLORREF COLOR_BLACK = RGB(0, 0, 0);
const COLORREF COLOR_DARK_GREY = RGB(20, 20, 20);
const COLORREF COLOR_GREY = RGB(40, 40, 40);
const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_BLUE = RGB(86, 156, 214);

HWND g_hMainWindow = nullptr;
HFONT g_hFont = nullptr;
int g_currentStep = 0;

void DrawTetrisLogo(HDC hdc, int x, int y, int size) {
    HBRUSH hBrush = CreateSolidBrush(COLOR_BLUE);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    Rectangle(hdc, x, y, x + size, y + size);
    Rectangle(hdc, x - size, y + size, x, y + size * 2);
    Rectangle(hdc, x, y + size, x + size, y + size * 2);
    Rectangle(hdc, x + size, y + size, x + size * 2, y + size * 2);
    
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

bool CheckVCRedist() {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64", 
        0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool InstallVCRedist() {
    HINTERNET hInternet = InternetOpenA("Aether", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hInternet) return false;
    
    HINTERNET hConnect = InternetOpenUrlA(hInternet, 
        "https://aka.ms/vs/17/release/vc_redist.x64.exe", 
        nullptr, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return false;
    }
    
    HANDLE hFile = CreateFileA("vc_redist.x64.exe", GENERIC_WRITE, 0, nullptr, 
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }
    
    char buffer[4096];
    DWORD bytesRead, bytesWritten;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        WriteFile(hFile, buffer, bytesRead, &bytesWritten, nullptr);
    }
    
    CloseHandle(hFile);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    
    if (CreateProcessA(nullptr, "vc_redist.x64.exe /quiet /norestart", 
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    
    DeleteFileA("vc_redist.x64.exe");
    return true;
}

LRESULT CALLBACK SetupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            g_hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            
            CreateWindowExW(0, L"BUTTON", L"Install",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                250, 300, 100, 35,
                hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr);
            break;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                SetWindowTextW(GetDlgItem(hwnd, 1001), L"Installing...");
                EnableWindow(GetDlgItem(hwnd, 1001), FALSE);
                
                if (!CheckVCRedist()) {
                    if (InstallVCRedist()) {
                        MessageBoxW(hwnd, L"Dependencies installed. Aether ready!", L"Setup", MB_OK);
                    } else {
                        MessageBoxW(hwnd, L"Installation failed. Manual install required.", L"Setup", MB_OK);
                    }
                } else {
                    MessageBoxW(hwnd, L"Aether installed successfully!", L"Setup", MB_OK);
                }
                
                PostQuitMessage(0);
            }
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            HBRUSH hBrush = CreateSolidBrush(COLOR_BLACK);
            FillRect(hdc, &clientRect, hBrush);
            DeleteObject(hBrush);
            
            DrawTetrisLogo(hdc, 50, 20, 12);
            
            SetTextColor(hdc, COLOR_WHITE);
            SetBkMode(hdc, TRANSPARENT);
            HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFont);
            TextOutW(hdc, 80, 25, L"Aether Setup", 12);
            TextOutW(hdc, 50, 100, L"Advanced Luau Scripting Environment", 35);
            TextOutW(hdc, 50, 130, L"This will install required dependencies", 39);
            TextOutW(hdc, 50, 160, L"and prepare Aether for use.", 27);
            SelectObject(hdc, hOldFont);
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_DESTROY:
            if (g_hFont) DeleteObject(g_hFont);
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    using namespace AetherSetup;
    
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = SetupWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_BLACK);
    wc.lpszClassName = L"AetherSetup";
    
    RegisterClassExW(&wc);
    
    HWND hWnd = CreateWindowExW(0, L"AetherSetup", L"Aether Setup",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        nullptr, nullptr, hInstance, nullptr);
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
