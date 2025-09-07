#include <windows.h>
#include <tlhelp32.h>

extern "C" {
    __declspec(dllexport) bool InjectIntoRoblox() {
        // Find Roblox process
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        
        PROCESSENTRY32W entry = {};
        entry.dwSize = sizeof(entry);
        
        bool found = false;
        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (wcscmp(entry.szExeFile, L"RobloxPlayerBeta.exe") == 0) {
                    found = true;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        
        CloseHandle(snapshot);
        return found;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
