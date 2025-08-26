#include "Core.h"
#include "AIController.h"
#include "PolymorphicEngine.h"
#include <vector>
#include "../kernel/bypasses/Injection.h" // Assuming access to kernel headers for enums/structs
#include <stdexcept>

#include <tlhelp32.h>

namespace AetherVisor {
    namespace Backend {
        // A simple script analyzer to check for potentially dangerous keywords.
        std::string AnalyzeScript(const std::string& script) {
            const std::vector<std::string> dangerousKeywords = {
                "io.open", "os.execute", "os.remove", "require", "dofile", "loadfile"
            };

            for (const auto& keyword : dangerousKeywords) {
                if (script.find(keyword) != std::string::npos) {
                    return "UNSAFE";
                }
            }
            return "SAFE";
        }
    }
}


// Helper to get a process ID by its name.
DWORD GetProcessIdByName(const std::wstring& processName) {
    PROCESSENTRY32W processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (processesSnapshot == INVALID_HANDLE_VALUE) return 0;

    Process32FirstW(processesSnapshot, &processInfo);
    do {
        if (wcscmp(processInfo.szExeFile, processName.c_str()) == 0) {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
    } while (Process32NextW(processesSnapshot, &processInfo));

    CloseHandle(processesSnapshot);
    return 0;
}

namespace AetherVisor {
    namespace Backend {

        Core& Core::GetInstance() {
            static Core instance;
            return instance;
        }

        bool Core::Initialize() {
            // TODO: Initialize backend services like IPC, load driver, etc.
            m_initialized = true;
            return true;
        }

        bool Core::Inject(const std::wstring& processName) {
            if (!m_initialized) return false;

            // For simplicity, we will now use a standard user-mode injection technique.
            // The "Conditional Injection" logic can be re-added later to choose
            // between different user-mode techniques if needed.

            // NOTE: The payload is no longer the DLL content, but the PATH to the DLL.
            char dllPath[MAX_PATH] = "AetherVisor.Payload.dll"; // Placeholder path

            DWORD processId = GetProcessIdByName(processName);
            if (processId == 0) return false;

            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
            if (hProcess == NULL) return false;

            // Allocate memory in the target process for the DLL path.
            LPVOID pDllPathRemote = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT, PAGE_READWRITE);
            if (pDllPathRemote == NULL) {
                CloseHandle(hProcess);
                return false;
            }

            // Write the DLL path into the allocated memory.
            if (!WriteProcessMemory(hProcess, pDllPathRemote, (LPVOID)dllPath, sizeof(dllPath), NULL)) {
                VirtualFreeEx(hProcess, pDllPathRemote, 0, MEM_RELEASE);
                CloseHandle(hProcess);
                return false;
            }

            // Get the address of LoadLibraryA.
            LPVOID pLoadLibraryA = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
            if (pLoadLibraryA == NULL) {
                 VirtualFreeEx(hProcess, pDllPathRemote, 0, MEM_RELEASE);
                CloseHandle(hProcess);
                return false;
            }

            // Create a remote thread that calls LoadLibraryA with our DLL path as an argument.
            HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, pDllPathRemote, 0, NULL);

            // Cleanup and return.
            bool success = (hRemoteThread != NULL);
            if (hRemoteThread) CloseHandle(hRemoteThread);
            VirtualFreeEx(hProcess, pDllPathRemote, 0, MEM_RELEASE);
            CloseHandle(hProcess);

            if (success) {
                AIController::GetInstance().ReportEvent(AIEventType::INJECTION_ATTEMPT);
            }

            return success;
        }

        void Core::Cleanup() {
            // Implements "Multi-Level Polymorphic Cleanup".
            // The order of cleanup operations is randomized to make the process less predictable.

            std::vector<std::function<void()>> cleanupTasks;

            // Add standard cleanup tasks.
            cleanupTasks.push_back([]{ Payload::EventManager::GetInstance().~EventManager(); });
            cleanupTasks.push_back([]{ Payload::NetworkManager::Uninstall(); });
            cleanupTasks.push_back([]{ Payload::MemoryPatcher::GetInstance().RevertAllPatches(); });

            // Add more intensive tasks if risk is high.
            auto& ai = AIController::GetInstance();
            if (ai.GetCurrentRiskLevel() >= RiskLevel::HIGH) {
                // Example of a high-risk cleanup task: scan for and zero out memory artifacts.
                cleanupTasks.push_back([]{ /* Zero out suspicious memory regions */ });
            }

            // Randomize the order of execution.
            auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::shuffle(cleanupTasks.begin(), cleanupTasks.end(), std::mt19937(static_cast<unsigned int>(seed)));

            // Execute all cleanup tasks.
            for (const auto& task : cleanupTasks) {
                task();
            }

            // TODO: Unload driver. This should typically be the very last step.

            m_targetProcess = nullptr;
            m_initialized = false;
        }

    } // namespace Backend
} // namespace AetherVisor
