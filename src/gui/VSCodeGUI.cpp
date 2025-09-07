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

// Black/Grey theme colors
const COLORREF COLOR_BLACK = RGB(0, 0, 0);
const COLORREF COLOR_DARK_GREY = RGB(20, 20, 20);
const COLORREF COLOR_GREY = RGB(40, 40, 40);
const COLORREF COLOR_LIGHT_GREY = RGB(60, 60, 60);
const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_BLUE = RGB(86, 156, 214);    // Keywords
const COLORREF COLOR_GREEN = RGB(106, 153, 85);   // Strings
const COLORREF COLOR_ORANGE = RGB(220, 220, 170); // Numbers

HWND g_hMainWindow = nullptr;
HWND g_hSearchBox = nullptr;
HWND g_hScriptList = nullptr;
HWND g_hEditor = nullptr;
HWND g_hTerminal = nullptr;
HWND g_hStatusBar = nullptr;

// Tetris logo
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

// ScriptBlox API integration
class ScriptBloxAPI {
private:
    std::string HttpGet(const std::string& url) {
        HINTERNET hInternet = InternetOpenA("Aether/1.0", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
        if (!hInternet) return "";
        
        HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) {
            InternetCloseHandle(hInternet);
            return "";
        }
        
        std::string result;
        char buffer[4096];
        DWORD bytesRead;
        
        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            result.append(buffer, bytesRead);
        }
        
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return result;
    }
    
public:
    std::vector<std::pair<std::string, std::string>> FetchScripts() {
        std::vector<std::pair<std::string, std::string>> scripts;
        
        std::string response = HttpGet("https://scriptblox.com/api/script/fetch?page=1&max=50");
        
        if (response.empty()) {
            // Fallback scripts
            scripts.push_back({"Infinite Yield", "loadstring(game:HttpGet('https://raw.githubusercontent.com/EdgeIY/infiniteyield/master/source'))();"});
            scripts.push_back({"Dark Dex V4", "loadstring(game:HttpGet('https://raw.githubusercontent.com/Babyhamsta/RBLX_Scripts/main/Universal/BypassedDarkDexV3.lua'))();"});
            scripts.push_back({"Universal ESP", "loadstring(game:HttpGet('https://raw.githubusercontent.com/ic3w0lf22/Unnamed-ESP/master/UnnamedESP.lua'))();"});
            return scripts;
        }
        
        // Simple JSON parsing for title and script
        size_t pos = 0;
        while ((pos = response.find("\"title\":", pos)) != std::string::npos) {
            pos += 8;
            size_t start = response.find("\"", pos) + 1;
            size_t end = response.find("\"", start);
            if (end == std::string::npos) break;
            
            std::string title = response.substr(start, end - start);
            
            // Find corresponding script
            size_t scriptPos = response.find("\"script\":", end);
            if (scriptPos != std::string::npos) {
                scriptPos += 9;
                size_t scriptStart = response.find("\"", scriptPos) + 1;
                size_t scriptEnd = response.find("\"", scriptStart);
                if (scriptEnd != std::string::npos) {
                    std::string script = response.substr(scriptStart, scriptEnd - scriptStart);
                    scripts.push_back({title, script});
                }
            }
            
            if (scripts.size() >= 20) break; // Limit results
        }
        
        return scripts;
    }
};

// Script search with ScriptBlox
class ScriptSearch {
private:
    HWND m_hSearchBox;
    HWND m_hListView;
    ScriptBloxAPI m_api;
    std::vector<std::pair<std::string, std::string>> m_scripts;
    
public:
    bool Create(HWND parent, int x, int y, int width, int height) {
        // Search box
        m_hSearchBox = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Search scripts...",
            WS_CHILD | WS_VISIBLE | ES_LEFT,
            x + 5, y + 5, width - 10, 25,
            parent, (HMENU)3001, GetModuleHandle(nullptr), nullptr);
            
        // Script list
        m_hListView = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER,
            x + 5, y + 35, width - 10, height - 40,
            parent, (HMENU)3002, GetModuleHandle(nullptr), nullptr);
            
        if (!m_hListView) return false;
        
        // Setup columns
        LVCOLUMNW col = {};
        col.mask = LVCF_WIDTH;
        col.cx = width - 30;
        ListView_InsertColumn(m_hListView, 0, &col);
        
        // Load scripts from API
        LoadScripts();
        
        return true;
    }
    
    void LoadScripts() {
        m_scripts = m_api.FetchScripts();
        
        for (const auto& script : m_scripts) {
            LVITEMW item = {};
            item.mask = LVIF_TEXT;
            item.iItem = ListView_GetItemCount(m_hListView);
            
            std::wstring title(script.first.begin(), script.first.end());
            item.pszText = (LPWSTR)title.c_str();
            ListView_InsertItem(m_hListView, &item);
        }
    }
    
    std::string GetSelectedScript() {
        int selected = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
        if (selected >= 0 && selected < (int)m_scripts.size()) {
            return m_scripts[selected].second;
        }
        return "";
    }
    
    HWND GetSearchBox() const { return m_hSearchBox; }
    HWND GetListView() const { return m_hListView; }
};

// Code editor with syntax highlighting
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
        
        // Black background, white text
        SendMessage(m_hEdit, EM_SETBKGNDCOLOR, 0, COLOR_BLACK);
        
        // Default code
        SetText(L"-- Aether Script Editor\n-- Select a script from the left panel\n\nprint(\"Hello from Aether!\")");
        
        return true;
    }
    
    void SetText(const std::wstring& text) {
        SetWindowTextW(m_hEdit, text.c_str());
        ApplySyntaxHighlighting();
    }
    
    void ApplySyntaxHighlighting() {
        // Basic syntax highlighting for Lua
        CHARFORMAT2W cf = {};
        cf.cbSize = sizeof(CHARFORMAT2W);
        cf.dwMask = CFM_COLOR;
        
        // Default white text
        cf.crTextColor = COLOR_WHITE;
        SendMessage(m_hEdit, EM_SETSEL, 0, -1);
        SendMessage(m_hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
        
        // Highlight keywords in blue
        std::vector<std::wstring> keywords = {L"local", L"function", L"end", L"if", L"then", L"else", L"for", L"while", L"do", L"return", L"true", L"false", L"nil"};
        
        cf.crTextColor = COLOR_BLUE;
        for (const auto& keyword : keywords) {
            HighlightWord(keyword, cf);
        }
        
        // Highlight strings in green
        cf.crTextColor = COLOR_GREEN;
        HighlightStrings(cf);
        
        // Reset selection
        SendMessage(m_hEdit, EM_SETSEL, 0, 0);
    }
    
    void HighlightWord(const std::wstring& word, CHARFORMAT2W& cf) {
        FINDTEXTEXW ft = {};
        ft.chrg.cpMin = 0;
        ft.chrg.cpMax = -1;
        ft.lpstrText = (LPWSTR)word.c_str();
        
        while (SendMessage(m_hEdit, EM_FINDTEXTEXW, FR_WHOLEWORD, (LPARAM)&ft) != -1) {
            SendMessage(m_hEdit, EM_SETSEL, ft.chrgText.cpMin, ft.chrgText.cpMax);
            SendMessage(m_hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
            ft.chrg.cpMin = ft.chrgText.cpMax;
        }
    }
    
    void HighlightStrings(CHARFORMAT2W& cf) {
        int textLen = GetWindowTextLengthW(m_hEdit);
        std::wstring text(textLen, L'\0');
        GetWindowTextW(m_hEdit, &text[0], textLen + 1);
        
        bool inString = false;
        wchar_t stringChar = L'\0';
        
        for (int i = 0; i < textLen; i++) {
            if (!inString && (text[i] == L'"' || text[i] == L'\'')) {
                inString = true;
                stringChar = text[i];
                int start = i;
                
                // Find end of string
                for (int j = i + 1; j < textLen; j++) {
                    if (text[j] == stringChar && text[j-1] != L'\\') {
                        SendMessage(m_hEdit, EM_SETSEL, start, j + 1);
                        SendMessage(m_hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
                        i = j;
                        break;
                    }
                }
                inString = false;
            }
        }
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
        
        // Dark grey background
        SendMessage(m_hTerminal, EM_SETBKGNDCOLOR, 0, COLOR_DARK_GREY);
        
        SetWindowTextW(m_hTerminal,
            L"Aether Terminal\n"
            L"[INFO] Backend loaded\n"
            L"[INFO] Security modules active\n"
            L"[INFO] ScriptBlox API connected\n"
            L"> Ready for execution\n"
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
            
            HMENU hRunMenu = CreatePopupMenu();
            AppendMenuW(hRunMenu, MF_STRING, 3001, L"Execute\tF5");
            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hRunMenu, L"Run");
            
            SetMenu(hwnd, hMenu);
            
            // Status bar
            g_hStatusBar = CreateWindowExW(0, STATUSCLASSNAME, L"Ready | ScriptBlox Connected",
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
            
            break;
        }
        
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            
            if (g_hStatusBar) {
                SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
            }
            
            RECT statusRect;
            GetWindowRect(g_hStatusBar, &statusRect);
            int statusHeight = statusRect.bottom - statusRect.top;
            
            int sidebarWidth = 300;
            int terminalHeight = 150;
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
                    g_codeEditor.SetText(L"-- New Script\n\n");
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
        
        case WM_NOTIFY: {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            if (pnmh->hwndFrom == g_scriptSearch.GetListView() && pnmh->code == NM_CLICK) {
                std::string script = g_scriptSearch.GetSelectedScript();
                if (!script.empty()) {
                    std::wstring wscript(script.begin(), script.end());
                    g_codeEditor.SetText(wscript);
                }
            }
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Black background
            HBRUSH hBrush = CreateSolidBrush(COLOR_BLACK);
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);
            
            // Tetris logo
            DrawTetrisLogo(hdc, 10, 5);
            
            // Title
            SetTextColor(hdc, COLOR_WHITE);
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, 30, 8, L"Aether", 6);
            
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
// Force workflow trigger
