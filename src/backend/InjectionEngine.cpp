#include "InjectionEngine.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
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

// NT API declarations
extern "C" {
    NTSTATUS NTAPI NtCreateThreadEx(
        PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, PVOID ObjectAttributes,
        HANDLE ProcessHandle, PVOID StartRoutine, PVOID Argument,
        ULONG CreateFlags, SIZE_T ZeroBits, SIZE_T StackSize, SIZE_T MaximumStackSize,
        PVOID AttributeList);
        
    NTSTATUS NTAPI NtAllocateVirtualMemory(
        HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize,
        ULONG AllocationType, ULONG Protect);
        
    NTSTATUS NTAPI NtWriteVirtualMemory(
        HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T BufferSize, PSIZE_T NumberOfBytesWritten);
}

class StealthInjector {
private:
    static bool IsDebuggerPresent() {
        // Multiple anti-debug checks
        if (::IsDebuggerPresent()) return true;
        
        // Check PEB flags
        PPEB peb = (PPEB)__readgsqword(0x60);
        if (peb->BeingDebugged) return true;
        if (peb->NtGlobalFlag & 0x70) return true;
        
        return false;
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
    
    static bool ValidatePE(const std::vector<BYTE>& peData) {
        if (peData.size() < sizeof(IMAGE_DOS_HEADER)) return false;
        
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)peData.data();
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;
        
        if (peData.size() < dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS)) return false;
        
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(peData.data() + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return false;
        
        return true;
    }
    
    static InjectionResult ManualMap(const std::vector<BYTE>& peData) {
        if (!ValidatePE(peData)) return InjectionResult::ERROR_INITIALIZATION_FAILED;
        
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)peData.data();
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(peData.data() + dosHeader->e_lfanew);
        
        // Allocate memory
        SIZE_T imageSize = ntHeaders->OptionalHeader.SizeOfImage;
        PVOID baseAddress = nullptr;
        
        NTSTATUS status = NtAllocateVirtualMemory(g_context.target_process, &baseAddress, 0, 
            &imageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (status != 0) return InjectionResult::ERROR_MEMORY_ALLOCATION_FAILED;
        
        // Copy headers
        SIZE_T bytesWritten;
        status = NtWriteVirtualMemory(g_context.target_process, baseAddress, 
            (PVOID)peData.data(), ntHeaders->OptionalHeader.SizeOfHeaders, &bytesWritten);
        if (status != 0) return InjectionResult::ERROR_MEMORY_ALLOCATION_FAILED;
        
        // Copy sections
        PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
        for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            if (sectionHeader[i].SizeOfRawData > 0) {
                PVOID sectionDest = (PVOID)((ULONG_PTR)baseAddress + sectionHeader[i].VirtualAddress);
                status = NtWriteVirtualMemory(g_context.target_process, sectionDest,
                    (PVOID)(peData.data() + sectionHeader[i].PointerToRawData),
                    sectionHeader[i].SizeOfRawData, &bytesWritten);
                if (status != 0) return InjectionResult::ERROR_MEMORY_ALLOCATION_FAILED;
            }
        }
        
        // Create thread
        PVOID entryPoint = (PVOID)((ULONG_PTR)baseAddress + ntHeaders->OptionalHeader.AddressOfEntryPoint);
        HANDLE hThread;
        status = NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, nullptr, g_context.target_process,
            entryPoint, baseAddress, 0, 0, 0, 0, nullptr);
        
        if (status != 0) return InjectionResult::ERROR_THREAD_CREATION_FAILED;
        
        if (hThread) {
            WaitForSingleObject(hThread, 5000);
            CloseHandle(hThread);
        }
        
        return InjectionResult::SUCCESS;
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
    
    static InjectionResult InjectDLL(const std::wstring& dllPath) {
        if (!g_context.target_process) return InjectionResult::ERROR_INITIALIZATION_FAILED;
        
        HANDLE hFile = CreateFileW(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return InjectionResult::ERROR_TARGET_NOT_FOUND;
        
        DWORD fileSize = GetFileSize(hFile, nullptr);
        std::vector<BYTE> dllData(fileSize);
        
        DWORD bytesRead;
        if (!ReadFile(hFile, dllData.data(), fileSize, &bytesRead, nullptr)) {
            CloseHandle(hFile);
            return InjectionResult::ERROR_INITIALIZATION_FAILED;
        }
        CloseHandle(hFile);
        
        return ManualMap(dllData);
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

// Public API
InjectionResult InjectionEngine::Initialize() {
    return StealthInjector::Initialize();
}

InjectionResult InjectionEngine::InjectIntoTarget(const std::wstring& targetProcess) {
    InjectionResult result = StealthInjector::AttachToProcess(targetProcess);
    if (result != InjectionResult::SUCCESS) return result;
    
    wchar_t modulePath[MAX_PATH];
    GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    
    std::wstring dllPath = modulePath;
    size_t pos = dllPath.find_last_of(L'\\');
    if (pos != std::wstring::npos) {
        dllPath = dllPath.substr(0, pos + 1) + L"aether_backend.dll";
    }
    
    return StealthInjector::InjectDLL(dllPath);
}

void InjectionEngine::Cleanup() {
    StealthInjector::Cleanup();
}

bool InjectionEngine::IsInitialized() {
    return g_context.initialized.load();
}

}
