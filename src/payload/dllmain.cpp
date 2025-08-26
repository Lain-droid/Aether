#include <Windows.h>
#include "PayloadIPC.h"
#include "../backend/Hooks.h"
#include "../backend/NetworkManager.h"
#include "../backend/MemoryPatcher.h"
#include <iostream>

namespace AetherVisor {
    namespace Payload {

        HMODULE g_hModule = NULL;

        void ShutdownPayload() {
            // This function is called when the backend signals a shutdown.
            // It cleans up all components from within the target process.
            Hooks::Uninstall();
            NetworkManager::Uninstall();
            MemoryPatcher::GetInstance().RevertAllPatches();

            // The final step is to unload the DLL itself.
            FreeLibraryAndExitThread(g_hModule, 0);
        }

        void MessageHandler(const Backend::IpcMessage& msg) {
            switch (msg.type) {
                case Backend::MessageType::Shutdown:
                    // Create a new thread to handle shutdown so we don't block the listener
                    // and can safely unload the DLL.
                    CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ShutdownPayload, NULL, 0, nullptr);
                    break;
                // Other message types from backend to payload could be handled here.
            }
        }

        void InitializePayload() {
            // This is where the payload would initialize its components.
            Hooks::Install(); // Hooks now write directly to a log file.

            // Start the IPC client to listen for commands from the backend.
            static PayloadIPC ipcClient;
            ipcClient.ConnectAndListen(MessageHandler);
        }
    }
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            AetherVisor::Payload::g_hModule = hModule;
            // Don't do heavy work in DllMain. Create a new thread.
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)AetherVisor::Payload::InitializePayload, hModule, 0, nullptr);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
