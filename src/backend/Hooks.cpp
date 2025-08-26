#include "Hooks.h"
#include "EventManager.h"
#include "security/XorStr.h"
#include <iostream> // For placeholder logging

// Assume we have a way to get the real addresses of Roblox's functions.
// These are placeholders. In a real scenario, this would involve signature scanning
// or getting them from the Luau state.
void* GetRobloxPrintAddress() {
    // Placeholder: In a real implementation, you would find this address dynamically.
    // For example, by getting a handle to a module and using GetProcAddress or scanning.
    return nullptr;
}

void* GetRobloxWarnAddress() {
    // Placeholder
    return nullptr;
}

namespace AetherVisor {
    namespace Payload {

        // Define the static members
        void* Hooks::m_originalPrintFunc = nullptr;
        void* Hooks::m_originalWarnFunc = nullptr;

        // The callback to send data back to the frontend
        static ConsoleOutputCallback g_consoleCallback = nullptr;

        // Our detour function for Roblox's 'print'
        void Detour_Print(const char* message) {
            if (g_consoleCallback) {
                g_consoleCallback(std::string(XorS("[PRINT]: ")) + message);
            }

            // Call the original 'print' function
            auto originalPrint = EventManager::GetInstance().GetOriginal<void(*)(const char*)>(GetRobloxPrintAddress());
            if (originalPrint) {
                originalPrint(message);
            }
        }

        // Our detour function for Roblox's 'warn'
        void Detour_Warn(const char* message) {
            if (g_consoleCallback) {
                g_consoleCallback(std::string(XorS("[WARN]: ")) + message);
            }

            // Call the original 'warn' function
            auto originalWarn = EventManager::GetInstance().GetOriginal<void(*)(const char*)>(GetRobloxWarnAddress());
            if (originalWarn) {
                originalWarn(message);
            }
        }

        bool Hooks::Install(ConsoleOutputCallback callback) {
            g_consoleCallback = callback;
            auto& eventManager = EventManager::GetInstance();

            void* printAddr = GetRobloxPrintAddress();
            void* warnAddr = GetRobloxWarnAddress();

            if (!printAddr || !warnAddr) {
                // In a real scenario, we'd log this failure.
                return false;
            }

            // Install hooks using the new EventManager
            bool printHooked = eventManager.Install(printAddr, (void*)Detour_Print);
            bool warnHooked = eventManager.Install(warnAddr, (void*)Detour_Warn);

            return printHooked && warnHooked;
        }

        void Hooks::Uninstall() {
            auto& eventManager = EventManager::GetInstance();

            void* printAddr = GetRobloxPrintAddress();
            void* warnAddr = GetRobloxWarnAddress();

            if (printAddr) {
                eventManager.Uninstall(printAddr);
            }
            if (warnAddr) {
                eventManager.Uninstall(warnAddr);
            }
            g_consoleCallback = nullptr;
        }

    } // namespace Payload
} // namespace AetherVisor
