#include "Hooks.h"
#include "EventManager.h"
#include "security/XorStr.h"
#include "security/SignatureScanner.h"
#include <fstream>
#include <string>

// Typedef for CreateProcessW so we can cast the original function pointer
typedef BOOL(WINAPI* CreateProcessW_t)(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);

// Finds the addresses of functions.
void* GetRobloxPrintAddress() {
    const char* pattern = "55 8B EC 6A ?? 68 ?? ?? ?? ?? 64 A1 ?? ?? ?? ?? 50 83 EC ?? 53 56 57 A1";
    return AetherVisor::Security::SignatureScanner::FindPattern(GetModuleHandle(NULL), pattern);
}
void* GetRobloxWarnAddress() {
    const char* pattern = "55 8B EC 83 E4 F8 83 EC ?? 56 8B F1 E8 ?? ?? ?? ?? 8B C8";
    return AetherVisor::Security::SignatureScanner::FindPattern(GetModuleHandle(NULL), pattern);
}
void* GetCreateProcessAddress() {
    // Using GetProcAddress for stable system APIs is fine.
    return GetProcAddress(GetModuleHandleA(XorS("kernel32.dll")), XorS("CreateProcessW"));
}

namespace AetherVisor {
    namespace Payload {

        static void* g_printAddress = nullptr;
        static void* g_warnAddress = nullptr;
        static void* g_createProcessAddress = nullptr;

        void WriteToLog(const std::string& message) {
            std::ofstream logfile("output.log", std::ios::app);
            if (logfile.is_open()) {
                logfile << message << std::endl;
            }
        }

        void Detour_Print(const char* message) {
            WriteToLog(std::string(XorS("[PRINT]: ")) + message);
            auto originalPrint = EventManager::GetInstance().GetOriginal<void(*)(const char*)>(g_printAddress);
            if (originalPrint) originalPrint(message);
        }

        void Detour_Warn(const char* message) {
            WriteToLog(std::string(XorS("[WARN]: ")) + message);
            auto originalWarn = EventManager::GetInstance().GetOriginal<void(*)(const char*)>(g_warnAddress);
            if (originalWarn) originalWarn(message);
        }

        BOOL WINAPI Detour_CreateProcessW(LPCWSTR app, LPWSTR cmd, LPSECURITY_ATTRIBUTES pAttr, LPSECURITY_ATTRIBUTES tAttr, BOOL inherit, DWORD flags, LPVOID env, LPCWSTR dir, LPSTARTUPINFOW si, LPPROCESS_INFORMATION pi) {
            WriteToLog(std::string(XorS("[HOOK]: CreateProcessW was called.")));
            auto originalFunc = EventManager::GetInstance().GetOriginal<CreateProcessW_t>(g_createProcessAddress);
            return originalFunc(app, cmd, pAttr, tAttr, inherit, flags, env, dir, si, pi);
        }

        bool Hooks::Install() {
            auto& eventManager = EventManager::GetInstance();

            g_printAddress = GetRobloxPrintAddress();
            g_warnAddress = GetRobloxWarnAddress();
            g_createProcessAddress = GetCreateProcessAddress();

            bool success = true;
            if (g_printAddress) { success &= eventManager.Install(g_printAddress, (void*)Detour_Print); }
            if (g_warnAddress) { success &= eventManager.Install(g_warnAddress, (void*)Detour_Warn); }
            if (g_createProcessAddress) { success &= eventManager.Install(g_createProcessAddress, (void*)Detour_CreateProcessW); }

            if (!success) {
                WriteToLog("Error: Failed to install one or more hooks.");
            }
            return success;
        }

        void Hooks::Uninstall() {
            auto& eventManager = EventManager::GetInstance();
            if (g_printAddress) eventManager.Uninstall(g_printAddress);
            if (g_warnAddress) eventManager.Uninstall(g_warnAddress);
            if (g_createProcessAddress) eventManager.Uninstall(g_createProcessAddress);
        }

    } // namespace Payload
} // namespace AetherVisor
