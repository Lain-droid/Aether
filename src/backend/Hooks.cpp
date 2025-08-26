#include "Hooks.h"
#include "EventManager.h"
#include "security/XorStr.h"
#include "security/SignatureScanner.h"
#include <fstream>
#include <string>

// Finds the addresses of Roblox functions using signature scanning.
void* GetRobloxPrintAddress() {
    const char* pattern = "55 8B EC 6A ?? 68 ?? ?? ?? ?? 64 A1 ?? ?? ?? ?? 50 83 EC ?? 53 56 57 A1";
    return AetherVisor::Security::SignatureScanner::FindPattern(GetModuleHandle(NULL), pattern);
}

void* GetRobloxWarnAddress() {
    const char* pattern = "55 8B EC 83 E4 F8 83 EC ?? 56 8B F1 E8 ?? ?? ?? ?? 8B C8";
    return AetherVisor::Security::SignatureScanner::FindPattern(GetModuleHandle(NULL), pattern);
}

namespace AetherVisor {
    namespace Payload {

        static void* g_printAddress = nullptr;
        static void* g_warnAddress = nullptr;

        // Helper to write to the log file.
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

        bool Hooks::Install() { // Removed callback parameter
            auto& eventManager = EventManager::GetInstance();

            g_printAddress = GetRobloxPrintAddress();
            g_warnAddress = GetRobloxWarnAddress();

            if (!g_printAddress || !g_warnAddress) {
                WriteToLog("Error: Could not find function addresses for hooks.");
                return false;
            }

            bool printHooked = eventManager.Install(g_printAddress, (void*)Detour_Print);
            bool warnHooked = eventManager.Install(g_warnAddress, (void*)Detour_Warn);

            return printHooked && warnHooked;
        }

        void Hooks::Uninstall() {
            auto& eventManager = EventManager::GetInstance();
            if (g_printAddress) eventManager.Uninstall(g_printAddress);
            if (g_warnAddress) eventManager.Uninstall(g_warnAddress);
        }

    } // namespace Payload
} // namespace AetherVisor
