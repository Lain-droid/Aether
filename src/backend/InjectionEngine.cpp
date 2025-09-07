#include "InjectionEngine.h"
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <atomic>

namespace Aether {

struct SecureContext {
    std::atomic<bool> initialized{false};
    HANDLE target_process{nullptr};
    DWORD target_pid{0};
    
    ~SecureContext() {
        if (target_process && target_process != INVALID_HANDLE_VALUE) {
            CloseHandle(target_process);
        }
    }
};

static SecureContext g_context;

class StealthInjector {
private:
    static bool IsDebuggerPresent() {
        return ::IsDebuggerPresent();
    }
    
    static DWORD FindProcess(const std::wstring& processName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;
        
        PROCESSENTRY32W entry = {};
        entry.dwSize = sizeof(entry);
        
        DWORD targetPid = 0;
        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (processName == entry.szExeFile) {
                    targetPid = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        
        CloseHandle(snapshot);
        return targetPid;
    }
    
public:
    static InjectionResult Initialize() {
        if (IsDebuggerPresent()) return InjectionResult::ERROR_INITIALIZATION_FAILED;
        g_context.initialized.store(true);
        return InjectionResult::SUCCESS;
    }
    
    static InjectionResult AttachToProcess(const std::wstring& processName) {
        if (!g_context.initialized.load()) return InjectionResult::ERROR_INITIALIZATION_FAILED;
        
        DWORD pid = FindProcess(processName);
        if (pid == 0) return InjectionResult::ERROR_TARGET_NOT_FOUND;
        
        HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!process) return InjectionResult::ERROR_ACCESS_DENIED;
        
        g_context.target_process = process;
        g_context.target_pid = pid;
        
        return InjectionResult::SUCCESS;
    }
    
    static void Cleanup() {
        g_context.initialized.store(false);
        if (g_context.target_process && g_context.target_process != INVALID_HANDLE_VALUE) {
            CloseHandle(g_context.target_process);
            g_context.target_process = nullptr;
        }
        g_context.target_pid = 0;
    }
};

InjectionResult InjectionEngine::Initialize() {
    return StealthInjector::Initialize();
}

InjectionResult InjectionEngine::InjectIntoTarget(const std::wstring& targetProcess) {
    return StealthInjector::AttachToProcess(targetProcess);
}

void InjectionEngine::Cleanup() {
    StealthInjector::Cleanup();
}

bool InjectionEngine::IsInitialized() {
    return g_context.initialized.load();
}

}
