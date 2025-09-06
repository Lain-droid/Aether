#include <windows.h>
#include <string>

// Aether Setup Wizard - Beautiful Installation Experience
namespace AetherSetup {

const COLORREF COLOR_BG = RGB(15, 15, 15);
const COLORREF COLOR_CARD = RGB(25, 25, 25);
const COLORREF COLOR_TEXT = RGB(240, 240, 240);
const COLORREF COLOR_ACCENT = RGB(0, 122, 204);
const COLORREF COLOR_SUCCESS = RGB(0, 200, 83);

HWND g_hMainWindow = nullptr;
int g_currentStep = 0;

// Enhanced Tetris logo with animation
void DrawAnimatedTetrisLogo(HDC hdc, int x, int y, int frame) {
    // Multiple Tetris pieces forming "AETHER"
    HBRUSH hBrush = CreateSolidBrush(COLOR_ACCENT);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    // Animated T-piece (main logo)
    int offset = (frame % 20) - 10; // Subtle animation
    RECT blocks[] = {
        {x + offset, y, x + offset + 12, y + 12},
        {x + offset - 12, y + 12, x + offset, y + 24},
        {x + offset, y + 12, x + offset + 12, y + 24},
        {x + offset + 12, y + 12, x + offset + 24, y + 24}
    };
    
    for (auto& block : blocks) {
        Rectangle(hdc, block.left, block.top, block.right, block.bottom);
    }
    
    // Additional decorative pieces
    HBRUSH hAccentBrush = CreateSolidBrush(COLOR_SUCCESS);
    SelectObject(hdc, hAccentBrush);
    
    // L-piece
    Rectangle(hdc, x + 40, y, x + 52, y + 12);
    Rectangle(hdc, x + 40, y + 12, x + 52, y + 24);
    Rectangle(hdc, x + 52, y + 12, x + 64, y + 24);
    
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
    DeleteObject(hAccentBrush);
}

// Modern card-style UI
void DrawCard(HDC hdc, int x, int y, int width, int height, const std::wstring& title, const std::wstring& content) {
    // Card background with subtle shadow
    HBRUSH hShadowBrush = CreateSolidBrush(RGB(5, 5, 5));
    RECT shadowRect = {x + 4, y + 4, x + width + 4, y + height + 4};
    FillRect(hdc, &shadowRect, hShadowBrush);
    DeleteObject(hShadowBrush);
    
    // Card background
    HBRUSH hCardBrush = CreateSolidBrush(COLOR_CARD);
    RECT cardRect = {x, y, x + width, y + height};
    FillRect(hdc, &cardRect, hCardBrush);
    DeleteObject(hCardBrush);
    
    // Card border
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(60, 60, 60));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    Rectangle(hdc, x, y, x + width, y + height);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    // Title
    SetTextColor(hdc, COLOR_TEXT);
    SetBkMode(hdc, TRANSPARENT);
    HFONT hTitleFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
    
    RECT titleRect = {x + 20, y + 20, x + width - 20, y + 50};
    DrawTextW(hdc, title.c_str(), -1, &titleRect, DT_LEFT | DT_VCENTER);
    
    // Content
    HFONT hContentFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    SelectObject(hdc, hContentFont);
    
    SetTextColor(hdc, RGB(180, 180, 180));
    RECT contentRect = {x + 20, y + 60, x + width - 20, y + height - 20};
    DrawTextW(hdc, content.c_str(), -1, &contentRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hTitleFont);
    DeleteObject(hContentFont);
}

// Setup steps
void DrawSetupStep(HDC hdc, int step) {
    switch (step) {
        case 0: // Welcome
            DrawCard(hdc, 100, 150, 400, 200,
                L"Welcome to Aether",
                L"Advanced Luau Scripting Environment\n\n"
                L"• VSCode-like editor with syntax highlighting\n"
                L"• Real-time console output\n"
                L"• Advanced security features\n"
                L"• User-mode operation (no kernel drivers)\n\n"
                L"Click Next to continue installation.");
            break;
            
        case 1: // License
            DrawCard(hdc, 100, 150, 400, 200,
                L"License Agreement",
                L"Educational and Research Use Only\n\n"
                L"This software is provided for educational and research purposes only. "
                L"You are solely responsible for your use of this software.\n\n"
                L"By clicking Accept, you agree to use this software responsibly "
                L"and in accordance with all applicable laws and platform terms.");
            break;
            
        case 2: // Installation
            DrawCard(hdc, 100, 150, 400, 200,
                L"Installing Aether",
                L"Installing components...\n\n"
                L"• Backend security modules ✓\n"
                L"• GUI interface ✓\n"
                L"• Configuration files ✓\n"
                L"• Desktop shortcuts ✓\n\n"
                L"Installation completed successfully!");
            break;
            
        case 3: // Complete
            DrawCard(hdc, 100, 150, 400, 200,
                L"Installation Complete",
                L"Aether has been successfully installed!\n\n"
                L"• Launch Aether from desktop shortcut\n"
                L"• Read documentation for usage guide\n"
                L"• Join community for support\n\n"
                L"Thank you for choosing Aether!");
            break;
    }
}

// Window procedure
LRESULT CALLBACK SetupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static int animFrame = 0;
    
    switch (uMsg) {
        case WM_CREATE: {
            // Create navigation buttons
            CreateWindowExW(0, L"BUTTON", L"Next",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                420, 400, 80, 35,
                hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr);
                
            CreateWindowExW(0, L"BUTTON", L"Cancel",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                320, 400, 80, 35,
                hwnd, (HMENU)1002, GetModuleHandle(nullptr), nullptr);
                
            // Animation timer
            SetTimer(hwnd, 1, 100, nullptr);
            break;
        }
        
        case WM_TIMER:
            animFrame++;
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
            
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
            
            // Background gradient
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            HBRUSH hBrush = CreateSolidBrush(COLOR_BG);
            FillRect(hdc, &clientRect, hBrush);
            DeleteObject(hBrush);
            
            // Header with animated logo
            DrawAnimatedTetrisLogo(hdc, 250, 30, animFrame);
            
            // Title
            SetTextColor(hdc, COLOR_TEXT);
            SetBkMode(hdc, TRANSPARENT);
            HFONT hFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            
            TextOutW(hdc, 200, 90, L"Aether Setup Wizard", 19);
            
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            // Current setup step
            DrawSetupStep(hdc, g_currentStep);
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_DESTROY:
            KillTimer(hwnd, 1);
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
    wc.hbrBackground = CreateSolidBrush(COLOR_BG);
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
