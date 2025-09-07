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
const COLORREF COLOR_DARK_GREY = RGB(15, 15, 15);
const COLORREF COLOR_GREY = RGB(30, 30, 30);
const COLORREF COLOR_LIGHT_GREY = RGB(45, 45, 45);
const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_BLUE = RGB(0, 120, 215);
const COLORREF COLOR_GREEN = RGB(16, 124, 16);

HWND g_hMainWindow = nullptr;
HWND g_hInstallBtn = nullptr;
HWND g_hProgressBar = nullptr;
HWND g_hStatusText = nullptr;
HFONT g_hFont = nullptr;
HFONT g_hTitleFont = nullptr;
HFONT g_hLargeFont = nullptr;
bool g_isInstalling = false;
int g_progress = 0;

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

void DrawGradientBackground(HDC hdc, RECT rect) {
    int height = rect.bottom - rect.top;
    for (int i = 0; i < height; i++) {
        int gray = 5 + (i * 10) / height;
        HBRUSH hBrush = CreateSolidBrush(RGB(gray, gray, gray));
        RECT lineRect = {rect.left, rect.top + i, rect.right, rect.top + i + 1};
        FillRect(hdc, &lineRect, hBrush);
        DeleteObject(hBrush);
    }
}

void DrawModernCard(HDC hdc, RECT rect, COLORREF color) {
    // Card shadow
    RECT shadowRect = {rect.left + 2, rect.top + 2, rect.right + 2, rect.bottom + 2};
    HBRUSH hShadowBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &shadowRect, hShadowBrush);
    DeleteObject(hShadowBrush);
    
    // Card background
    HBRUSH hCardBrush = CreateSolidBrush(color);
    FillRect(hdc, &rect, hCardBrush);
    DeleteObject(hCardBrush);
    
    // Card border
    HPEN hPen = CreatePen(PS_SOLID, 1, COLOR_LIGHT_GREY);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
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

DWORD WINAPI InstallThread(LPVOID lpParam) {
    HWND hwnd = (HWND)lpParam;
    
    // Update status
    SetWindowTextW(g_hStatusText, L"Checking system requirements...");
    SendMessage(g_hProgressBar, PBM_SETPOS, 20, 0);
    Sleep(1000);
    
    if (!CheckVCRedist()) {
        SetWindowTextW(g_hStatusText, L"Downloading runtime components...");
        SendMessage(g_hProgressBar, PBM_SETPOS, 40, 0);
        
        HINTERNET hInternet = InternetOpenA("Aether", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
        if (hInternet) {
            HINTERNET hConnect = InternetOpenUrlA(hInternet, 
                "https://aka.ms/vs/17/release/vc_redist.x64.exe", 
                nullptr, 0, INTERNET_FLAG_RELOAD, 0);
            if (hConnect) {
                SetWindowTextW(g_hStatusText, L"Installing runtime components...");
                SendMessage(g_hProgressBar, PBM_SETPOS, 70, 0);
                
                HANDLE hFile = CreateFileA("vc_redist.x64.exe", GENERIC_WRITE, 0, nullptr, 
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (hFile != INVALID_HANDLE_VALUE) {
                    char buffer[4096];
                    DWORD bytesRead, bytesWritten;
                    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
                        WriteFile(hFile, buffer, bytesRead, &bytesWritten, nullptr);
                    }
                    CloseHandle(hFile);
                    
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
                }
                InternetCloseHandle(hConnect);
            }
            InternetCloseHandle(hInternet);
        }
    }
    
    SetWindowTextW(g_hStatusText, L"Finalizing installation...");
    SendMessage(g_hProgressBar, PBM_SETPOS, 90, 0);
    Sleep(1000);
    
    SetWindowTextW(g_hStatusText, L"Installation completed successfully");
    SendMessage(g_hProgressBar, PBM_SETPOS, 100, 0);
    
    SetWindowTextW(g_hInstallBtn, L"Finish");
    EnableWindow(g_hInstallBtn, TRUE);
    g_isInstalling = false;
    
    return 0;
}

LRESULT CALLBACK SetupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Create fonts
            g_hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
                
            g_hTitleFont = CreateFontW(24, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
                
            g_hLargeFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            
            // Install button
            g_hInstallBtn = CreateWindowExW(0, L"BUTTON", L"Install",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
                200, 320, 120, 40,
                hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr);
            SendMessage(g_hInstallBtn, WM_SETFONT, (WPARAM)g_hLargeFont, TRUE);
            
            // Progress bar
            g_hProgressBar = CreateWindowExW(0, PROGRESS_CLASS, nullptr,
                WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                80, 280, 360, 20,
                hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
            SendMessage(g_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            
            // Status text
            g_hStatusText = CreateWindowExW(0, L"STATIC", L"Ready to install",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                80, 250, 360, 25,
                hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
            SendMessage(g_hStatusText, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            
            break;
        }
        
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            if (dis->CtlID == 1001) {
                // Modern button design
                COLORREF btnColor = g_isInstalling ? COLOR_GREY : COLOR_BLUE;
                
                HBRUSH hBrush = CreateSolidBrush(btnColor);
                FillRect(dis->hDC, &dis->rcItem, hBrush);
                DeleteObject(hBrush);
                
                // Button text
                SetTextColor(dis->hDC, COLOR_WHITE);
                SetBkMode(dis->hDC, TRANSPARENT);
                HFONT hOldFont = (HFONT)SelectObject(dis->hDC, g_hLargeFont);
                
                wchar_t text[32];
                GetWindowTextW(dis->hwndItem, text, 32);
                
                RECT textRect = dis->rcItem;
                DrawTextW(dis->hDC, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                SelectObject(dis->hDC, hOldFont);
            }
            return TRUE;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                if (!g_isInstalling) {
                    if (wcscmp(L"Finish", (wchar_t*)SendMessage(g_hInstallBtn, WM_GETTEXT, 32, (LPARAM)L"")) == 0) {
                        PostQuitMessage(0);
                    } else {
                        g_isInstalling = true;
                        SetWindowTextW(g_hInstallBtn, L"Installing...");
                        EnableWindow(g_hInstallBtn, FALSE);
                        SendMessage(g_hProgressBar, PBM_SETPOS, 0, 0);
                        
                        DWORD threadId;
                        CreateThread(nullptr, 0, InstallThread, hwnd, 0, &threadId);
                    }
                }
            }
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            // Gradient background
            DrawGradientBackground(hdc, clientRect);
            
            // Main card
            RECT cardRect = {60, 60, clientRect.right - 60, 220};
            DrawModernCard(hdc, cardRect, COLOR_DARK_GREY);
            
            // Header area
            RECT headerRect = {0, 0, clientRect.right, 50};
            HBRUSH hHeaderBrush = CreateSolidBrush(COLOR_BLACK);
            FillRect(hdc, &headerRect, hHeaderBrush);
            DeleteObject(hHeaderBrush);
            
            // Tetris logo
            DrawTetrisLogo(hdc, 30, 15, 8);
            
            // Title
            SetTextColor(hdc, COLOR_WHITE);
            SetBkMode(hdc, TRANSPARENT);
            HFONT hOldFont = (HFONT)SelectObject(hdc, g_hTitleFont);
            TextOutW(hdc, 60, 18, L"Aether", 6);
            SelectObject(hdc, hOldFont);
            
            // Card content
            hOldFont = (HFONT)SelectObject(hdc, g_hLargeFont);
            SetTextColor(hdc, COLOR_WHITE);
            TextOutW(hdc, 100, 100, L"Advanced Luau Scripting Environment", 35);
            
            SelectObject(hdc, g_hFont);
            SetTextColor(hdc, RGB(180, 180, 180));
            TextOutW(hdc, 100, 130, L"Professional-grade security and performance", 43);
            TextOutW(hdc, 100, 150, L"User-mode operation with advanced evasion", 41);
            TextOutW(hdc, 100, 170, L"Modern VSCode-inspired interface", 32);
            TextOutW(hdc, 100, 190, L"Zero external dependencies after setup", 38);
            
            SelectObject(hdc, hOldFont);
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, COLOR_WHITE);
            SetBkColor(hdcStatic, COLOR_BLACK);
            return (INT_PTR)CreateSolidBrush(COLOR_BLACK);
        }
        
        case WM_DESTROY:
            if (g_hFont) DeleteObject(g_hFont);
            if (g_hTitleFont) DeleteObject(g_hTitleFont);
            if (g_hLargeFont) DeleteObject(g_hLargeFont);
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
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = SetupWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_BLACK);
    wc.lpszClassName = L"AetherSetup";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    
    RegisterClassExW(&wc);
    
    HWND hWnd = CreateWindowExW(
        WS_EX_LAYERED, L"AetherSetup", L"Aether Setup",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 420,
        nullptr, nullptr, hInstance, nullptr
    );
    
    // Window transparency effect
    SetLayeredWindowAttributes(hWnd, 0, 250, LWA_ALPHA);
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
