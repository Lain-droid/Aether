#include <windows.h>

LRESULT CALLBACK SetupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateWindowExW(0, L"BUTTON", L"Install", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                100, 100, 80, 30, hwnd, (HMENU)1001, GetModuleHandle(nullptr), nullptr);
            break;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                MessageBoxW(hwnd, L"Aether installed successfully!", L"Setup", MB_OK);
                PostQuitMessage(0);
            }
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = SetupWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"AetherSetup";
    
    RegisterClassExW(&wc);
    
    HWND hWnd = CreateWindowExW(0, L"AetherSetup", L"Aether Setup",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
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
