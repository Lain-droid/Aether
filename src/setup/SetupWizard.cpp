#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>
#include <string>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

namespace AetherSetup {

// Same colors as GUI - Black/Grey theme
const COLORREF COLOR_BLACK = RGB(0, 0, 0);
const COLORREF COLOR_DARK_GREY = RGB(20, 20, 20);
const COLORREF COLOR_GREY = RGB(40, 40, 40);
const COLORREF COLOR_LIGHT_GREY = RGB(60, 60, 60);
const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_BLUE = RGB(86, 156, 214);

HWND g_hMainWindow = nullptr;
HFONT g_hFont = nullptr;
HFONT g_hTitleFont = nullptr;
int g_currentStep = 0;

// Same Tetris logo as GUI
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

// Setup steps with black/grey theme
void DrawSetupStep(HDC hdc, int step, RECT clientRect) {
    int centerX = clientRect.right / 2;
    int centerY = clientRect.bottom / 2;
    
    // Setup card background
    HBRUSH hCardBrush = CreateSolidBrush(COLOR_DARK_GREY);
    RECT cardRect = {centerX - 200, centerY - 100, centerX + 200, centerY + 100};
    FillRect(hdc, &cardRect, hCardBrush);
    DeleteObject(hCardBrush);
    
    // Card border
    HPEN hPen = CreatePen(PS_SOLID, 1, COLOR_GREY);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    Rectangle(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    // Text setup
    SetTextColor(hdc, COLOR_WHITE);
    SetBkMode(hdc, TRANSPARENT);
    
    // Title font
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hTitleFont);
    
    switch (step) {
        case 0: // Welcome
            TextOutW(hdc, centerX - 80, centerY - 80, L"Welcome to Aether", 18);
            SelectObject(hdc, g_hFont);
            TextOutW(hdc, centerX - 150, centerY - 50, L"Advanced Luau Scripting Environment", 35);
            TextOutW(hdc, centerX - 120, centerY - 30, L"• Black/Grey VSCode-like interface", 34);
            TextOutW(hdc, centerX - 100, centerY - 10, L"• ScriptBlox API integration", 28);
            TextOutW(hdc, centerX - 80, centerY + 10, L"• 9.8/10 Security rating", 24);
            TextOutW(hdc, centerX - 90, centerY + 30, L"• User-mode operation", 21);
            TextOutW(hdc, centerX - 80, centerY + 60, L"Click Next to continue", 22);
            break;
            
        case 1: // License
            TextOutW(hdc, centerX - 70, centerY - 80, L"License Agreement", 17);
            SelectObject(hdc, g_hFont);
            TextOutW(hdc, centerX - 140, centerY - 50, L"Educational and Research Use Only", 33);
            TextOutW(hdc, centerX - 160, centerY - 20, L"This software is for educational purposes only.", 46);
            TextOutW(hdc, centerX - 150, centerY, L"You are responsible for your usage.", 35);
            TextOutW(hdc, centerX - 120, centerY + 30, L"By clicking Accept, you agree to", 32);
            TextOutW(hdc, centerX - 100, centerY + 50, L"use this software responsibly.", 30);
            break;
            
        case 2: // Installation
            TextOutW(hdc, centerX - 80, centerY - 80, L"Installing Aether", 17);
            SelectObject(hdc, g_hFont);
            TextOutW(hdc, centerX - 100, centerY - 50, L"Installing components...", 24);
            TextOutW(hdc, centerX - 120, centerY - 20, L"✓ Backend security modules", 26);
            TextOutW(hdc, centerX - 100, centerY, L"✓ VSCode-like GUI", 17);
            TextOutW(hdc, centerX - 90, centerY + 20, L"✓ Tetris branding", 17);
            TextOutW(hdc, centerX - 80, centerY + 40, L"✓ Configuration", 15);
            TextOutW(hdc, centerX - 70, centerY + 70, L"Installation complete!", 21);
            break;
            
        case 3: // Complete
            TextOutW(hdc, centerX - 90, centerY - 80, L"Installation Complete", 21);
            SelectObject(hdc, g_hFont);
            TextOutW(hdc, centerX - 120, centerY - 50, L"Aether has been installed!", 26);
            TextOutW(hdc, centerX - 100, centerY - 20, L"• Launch from desktop", 21);
            TextOutW(hdc, centerX - 90, centerY, L"• Enjoy black/grey theme", 24);
            TextOutW(hdc, centerX - 80, centerY + 20, L"• Use ScriptBlox API", 20);
            TextOutW(hdc, centerX - 70, centerY + 40, L"• Stay secure!", 14);
            TextOutW(hdc, centerX - 80, centerY + 70, L"Thank you for choosing Aether!", 30);
            break;
    }
    
    SelectObject(hdc, hOldFont);
}

// Progress bar
void DrawProgressBar(HDC hdc, int step, RECT clientRect) {
    int barWidth = 300;
    int barHeight = 8;
    int barX = (clientRect.right - barWidth) / 2;
    int barY = clientRect.bottom - 80;
    
    // Background
    HBRUSH hBgBrush = CreateSolidBrush(COLOR_GREY);
    RECT bgRect = {barX, barY, barX + barWidth, barY + barHeight};
    FillRect(hdc, &bgRect, hBgBrush);
    DeleteObject(hBgBrush);
    
    // Progress
    int progress = (step * barWidth) / 3;
    HBRUSH hProgressBrush = CreateSolidBrush(COLOR_BLUE);
    RECT progressRect = {barX, barY, barX + progress, barY + barHeight};
    FillRect(hdc, &progressRect, hProgressBrush);
    DeleteObject(hProgressBrush);
}

// Window procedure
LRESULT CALLBACK SetupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Create fonts (same as GUI)
            g_hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
                
            g_hTitleFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            
            // Navigation buttons with dark theme
            HWND hNextBtn = CreateWindowExW(0, L"BUTTON", L"Next",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
                420, 400, 80, 35,
                hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr);
                
            HWND hCancelBtn = CreateWindowExW(0, L"BUTTON", L"Cancel",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
                320, 400, 80, 35,
                hwnd, (HMENU)1002, GetModuleHandle(nullptr), nullptr);
            
            break;
        }
        
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            if (dis->CtlType == ODT_BUTTON) {
                // Custom button drawing with dark theme
                HBRUSH hBrush = CreateSolidBrush(COLOR_GREY);
                FillRect(dis->hDC, &dis->rcItem, hBrush);
                DeleteObject(hBrush);
                
                // Button border
                HPEN hPen = CreatePen(PS_SOLID, 1, COLOR_LIGHT_GREY);
                HPEN hOldPen = (HPEN)SelectObject(dis->hDC, hPen);
                Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top, 
                         dis->rcItem.right, dis->rcItem.bottom);
                SelectObject(dis->hDC, hOldPen);
                DeleteObject(hPen);
                
                // Button text
                SetTextColor(dis->hDC, COLOR_WHITE);
                SetBkMode(dis->hDC, TRANSPARENT);
                HFONT hOldFont = (HFONT)SelectObject(dis->hDC, g_hFont);
                
                wchar_t text[32];
                GetWindowTextW(dis->hwndItem, text, 32);
                
                RECT textRect = dis->rcItem;
                DrawTextW(dis->hDC, text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                SelectObject(dis->hDC, hOldFont);
            }
            return TRUE;
        }
        
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 1001: // Next
                    if (g_currentStep < 3) {
                        g_currentStep++;
                        InvalidateRect(hwnd, nullptr, TRUE);
                        
                        if (g_currentStep == 3) {
                            SetWindowTextW(GetDlgItem(hwnd, 1001), L"Finish");
                        }
                    } else {
                        PostQuitMessage(0);
                    }
                    break;
                    
                case 1002: // Cancel
                    PostQuitMessage(0);
                    break;
            }
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            // Black background (same as GUI)
            HBRUSH hBrush = CreateSolidBrush(COLOR_BLACK);
            FillRect(hdc, &clientRect, hBrush);
            DeleteObject(hBrush);
            
            // Header with Tetris logo
            DrawTetrisLogo(hdc, 50, 20, 12);
            
            // Title
            SetTextColor(hdc, COLOR_WHITE);
            SetBkMode(hdc, TRANSPARENT);
            HFONT hOldFont = (HFONT)SelectObject(hdc, g_hTitleFont);
            TextOutW(hdc, 80, 25, L"Aether Setup Wizard", 19);
            SelectObject(hdc, hOldFont);
            
            // Current setup step
            DrawSetupStep(hdc, g_currentStep, clientRect);
            
            // Progress bar
            DrawProgressBar(hdc, g_currentStep, clientRect);
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_DESTROY:
            if (g_hFont) DeleteObject(g_hFont);
            if (g_hTitleFont) DeleteObject(g_hTitleFont);
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

} // namespace AetherSetup

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    using namespace AetherSetup;
    
    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = SetupWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_BLACK);
    wc.lpszClassName = L"AetherSetup";
    
    RegisterClassExW(&wc);
    
    // Create setup window
    HWND hWnd = CreateWindowExW(
        0, L"AetherSetup", L"Aether Setup Wizard",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!hWnd) return -1;
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
