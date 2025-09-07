#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

namespace AetherGUI {

// VSCode colors
const COLORREF COLOR_TITLEBAR = RGB(60, 60, 60);
const COLORREF COLOR_SIDEBAR = RGB(37, 37, 38);
const COLORREF COLOR_EDITOR = RGB(30, 30, 30);
const COLORREF COLOR_PANEL = RGB(25, 25, 25);
const COLORREF COLOR_TEXT = RGB(212, 212, 212);
const COLORREF COLOR_ACCENT = RGB(0, 122, 204);
const COLORREF COLOR_BORDER = RGB(69, 69, 69);

HWND g_hMainWindow = nullptr;
HWND g_hSidebar = nullptr;
HWND g_hSearchBox = nullptr;
HWND g_hScriptList = nullptr;
HWND g_hTabControl = nullptr;
HWND g_hEditor = nullptr;
HWND g_hTerminal = nullptr;
HWND g_hStatusBar = nullptr;

// Tetris logo
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

// Script search sidebar
class ScriptSearch {
private:
    HWND m_hSearchBox;
    HWND m_hListView;
    
public:
    bool Create(HWND parent, int x, int y, int width, int height) {
        // Search box
        m_hSearchBox = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Search scripts...",
            WS_CHILD | WS_VISIBLE | ES_LEFT,
            x + 10, y + 10, width - 20, 25,
            parent, (HMENU)3001, GetModuleHandle(nullptr), nullptr);
            
        // Script list
        m_hListView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER,
            x + 10, y + 45, width - 20, height - 55,
            parent, (HMENU)3002, GetModuleHandle(nullptr), nullptr);
            
        if (!m_hListView) return false;
        
        // Setup list columns
        LVCOLUMNW col = {};
        col.mask = LVCF_WIDTH;
        col.cx = width - 40;
        ListView_InsertColumn(m_hListView, 0, &col);
        
        // Add sample scripts
        AddScript(L"Infinite Yield");
        AddScript(L"Dark Dex V4");
        AddScript(L"Arsenal Aimbot");
        AddScript(L"Blox Fruits Auto Farm");
        AddScript(L"Adopt Me Auto Pet");
        AddScript(L"Jailbreak Auto Rob");
        AddScript(L"MM2 ESP");
        AddScript(L"Phantom Forces Aimbot");
        AddScript(L"Prison Life GUI");
        AddScript(L"Brookhaven RP GUI");
        
        return true;
    }
    
    void AddScript(const std::wstring& name) {
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = ListView_GetItemCount(m_hListView);
        item.pszText = (LPWSTR)name.c_str();
        ListView_InsertItem(m_hListView, &item);
    }
    
    HWND GetSearchBox() const { return m_hSearchBox; }
    HWND GetListView() const { return m_hListView; }
};

// VSCode editor
class CodeEditor {
private:
    HWND m_hEdit;
    
public:
    bool Create(HWND parent, int x, int y, int width, int height) {
        LoadLibraryW(L"riched20.dll");
        
        m_hEdit = CreateWindowExW(0, RICHEDIT_CLASSW, L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_WANTRETURN,
            x, y, width, height,
            parent, (HMENU)4001, GetModuleHandle(nullptr), nullptr);
            
        if (!m_hEdit) return false;
        
        SendMessage(m_hEdit, EM_SETBKGNDCOLOR, 0, COLOR_EDITOR);
        
        // Default code
        SetWindowTextW(m_hEdit, 
            L"-- Aether Script Editor\n"
            L"-- Select a script from the left panel\n\n"
            L"local Players = game:GetService(\"Players\")\n"
            L"local player = Players.LocalPlayer\n\n"
            L"print(\"Hello from Aether!\")\n"
            L"print(\"Player:\", player.Name)\n\n"
            L"-- Your script will appear here when selected\n"
        );
        
        return true;
    }
    
    HWND GetHandle() const { return m_hEdit; }
};

// Terminal
class Terminal {
private:
    HWND m_hTerminal;
    
public:
    bool Create(HWND parent, int x, int y, int width, int height) {
        m_hTerminal = CreateWindowExW(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
            x, y, width, height,
            parent, (HMENU)5001, GetModuleHandle(nullptr), nullptr);
            
        if (!m_hTerminal) return false;
        
        SendMessage(m_hTerminal, EM_SETBKGNDCOLOR, 0, COLOR_PANEL);
        
        SetWindowTextW(m_hTerminal,
            L"Aether Terminal\n"
            L"[INFO] Backend loaded\n"
            L"[INFO] Security active\n"
            L"[INFO] Ready for execution\n"
            L"> _"
        );
        
        return true;
    }
    
    HWND GetHandle() const { return m_hTerminal; }
};

// Global instances
ScriptSearch g_scriptSearch;
CodeEditor g_codeEditor;
Terminal g_terminal;

// Main window procedure
LRESULT CALLBACK VSCodeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            InitCommonControls();
            
            // Create menu
            HMENU hMenu = CreateMenu();
            HMENU hFileMenu = CreatePopupMenu();
            AppendMenuW(hFileMenu, MF_STRING, 1001, L"New\tCtrl+N");
            AppendMenuW(hFileMenu, MF_STRING, 1002, L"Open\tCtrl+O");
            AppendMenuW(hFileMenu, MF_STRING, 1003, L"Save\tCtrl+S");
            AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(hFileMenu, MF_STRING, 1004, L"Exit");
            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
            
            HMENU hEditMenu = CreatePopupMenu();
            AppendMenuW(hEditMenu, MF_STRING, 2001, L"Cut\tCtrl+X");
            AppendMenuW(hEditMenu, MF_STRING, 2002, L"Copy\tCtrl+C");
            AppendMenuW(hEditMenu, MF_STRING, 2003, L"Paste\tCtrl+V");
            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, L"Edit");
            
            HMENU hRunMenu = CreatePopupMenu();
            AppendMenuW(hRunMenu, MF_STRING, 3001, L"Execute\tF5");
            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hRunMenu, L"Run");
            
            SetMenu(hwnd, hMenu);
            
            // Status bar
            g_hStatusBar = CreateWindowExW(0, STATUSCLASSNAME, L"Ready",
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
            
            break;
        }
        
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            
            // Resize status bar
            if (g_hStatusBar) {
                SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
            }
            
            RECT statusRect;
            GetWindowRect(g_hStatusBar, &statusRect);
            int statusHeight = statusRect.bottom - statusRect.top;
            
            // Layout
            int sidebarWidth = 300;
            int terminalHeight = 200;
            int menuHeight = 25;
            
            // Script search sidebar
            g_scriptSearch.Create(hwnd, 0, menuHeight, sidebarWidth, height - menuHeight - statusHeight);
            
            // Editor
            g_codeEditor.Create(hwnd, sidebarWidth, menuHeight, 
                              width - sidebarWidth, height - menuHeight - terminalHeight - statusHeight);
            
            // Terminal
            g_terminal.Create(hwnd, sidebarWidth, height - terminalHeight - statusHeight,
                            width - sidebarWidth, terminalHeight);
            
            break;
        }
        
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 1001: // New
                    SetWindowTextW(g_codeEditor.GetHandle(), L"-- New Script\n\n");
                    break;
                    
                case 3001: // Execute
                    MessageBoxW(hwnd, L"Script executed!", L"Aether", MB_OK);
                    break;
                    
                case 1004: // Exit
                    PostQuitMessage(0);
                    break;
            }
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Background
            HBRUSH hBrush = CreateSolidBrush(COLOR_EDITOR);
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);
            
            // Tetris logo
            DrawTetrisLogo(hdc, 15, 5);
            
            // Title
            SetTextColor(hdc, COLOR_TEXT);
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, 40, 8, L"Aether", 6);
            
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
        wc.hbrBackground = CreateSolidBrush(COLOR_EDITOR);
        wc.lpszClassName = L"AetherVSCode";
        
        RegisterClassExW(&wc);
        
        g_hMainWindow = CreateWindowExW(0, L"AetherVSCode", L"Aether - VSCode Interface",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,
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
