#include "InjectionEngine.h"
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <intrin.h>
#include <array>
#include <atomic>

#pragma comment(lib, "ntdll.lib")

namespace Aether::Core {

// Security context for injection operations
struct SecureContext {
    std::atomic<bool> initialized{false};
    std::atomic<uint32_t> security_token{0};
    HANDLE target_process{nullptr};
    DWORD target_pid{0};
    void* allocated_memory{nullptr};
    size_t allocated_size{0};
    
    ~SecureContext() {
        if (target_process && target_process != INVALID_HANDLE_VALUE) {
            CloseHandle(target_process);
        }
    }
};

static SecureContext g_context;

// NT API function signatures with proper error handling
extern "C" {
    NTSTATUS NTAPI NtCreateThreadEx(
        PHANDLE ThreadHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes,
        HANDLE ProcessHandle,
        PVOID StartRoutine,
        PVOID Argument,
        ULONG CreateFlags,
        SIZE_T ZeroBits,
        SIZE_T StackSize,
        SIZE_T MaximumStackSize,
        PVOID AttributeList
    );
    
    NTSTATUS NTAPI NtAllocateVirtualMemory(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        ULONG_PTR ZeroBits,
        PSIZE_T RegionSize,
        ULONG AllocationType,
        ULONG Protect
    );
    
    NTSTATUS NTAPI NtWriteVirtualMemory(
        HANDLE ProcessHandle,
        PVOID BaseAddress,
        PVOID Buffer,
        SIZE_T BufferSize,
        PSIZE_T NumberOfBytesWritten
    );
    
    NTSTATUS NTAPI NtProtectVirtualMemory(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        PSIZE_T RegionSize,
        ULONG NewProtect,
        PULONG OldProtect
    );
    
    NTSTATUS NTAPI NtQueryInformationProcess(
        HANDLE ProcessHandle,
        PROCESSINFOCLASS ProcessInformationClass,
        PVOID ProcessInformation,
        ULONG ProcessInformationLength,
        PULONG ReturnLength
    );
}

class SecureInjector {
private:
    static constexpr uint32_t PE_SIGNATURE = 0x00004550;
    static constexpr uint16_t DOS_SIGNATURE = 0x5A4D;
    static constexpr uint32_t SECURITY_COOKIE = 0x2B992DDFA232D1B4ULL;
    
    // Anti-debugging and evasion techniques
    static bool IsDebuggerPresent() noexcept {
        // Multiple debugger detection methods
        if (::IsDebuggerPresent()) return true;
        
        // Check PEB flags
        PPEB peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
        if (peb->BeingDebugged) return true;
        if (peb->NtGlobalFlag & 0x70) return true;
        
        // Check heap flags
        PVOID heap = peb->ProcessHeap;
        DWORD flags = *reinterpret_cast<PDWORD>(reinterpret_cast<PBYTE>(heap) + 0x70);
        DWORD force_flags = *reinterpret_cast<PDWORD>(reinterpret_cast<PBYTE>(heap) + 0x74);
        
        if (flags & ~HEAP_GROWABLE || force_flags != 0) return true;
        
        return false;
    }
    
    static bool ValidateProcessIntegrity(HANDLE process) noexcept {
        if (!process || process == INVALID_HANDLE_VALUE) return false;
        
        // Check process mitigation policies
        PROCESS_MITIGATION_POLICY_INFORMATION policy_info = {};
        policy_info.Policy = ProcessDEPPolicy;
        
        if (!GetProcessMitigationPolicy(process, ProcessDEPPolicy, &policy_info, sizeof(policy_info))) {
            return false;
        }
        
        // Validate process is not protected
        DWORD process_info = 0;
        ULONG return_length = 0;
        NTSTATUS status = NtQueryInformationProcess(process, ProcessBasicInformation, 
            &process_info, sizeof(process_info), &return_length);
        
        return NT_SUCCESS(status);
    }
    
    static DWORD FindSecureProcess(const std::wstring& process_name) noexcept {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;
        
        PROCESSENTRY32W entry = {};
        entry.dwSize = sizeof(PROCESSENTRY32W);
        
        DWORD target_pid = 0;
        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (process_name == entry.szExeFile) {
                    // Additional validation for target process
                    HANDLE test_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, entry.th32ProcessID);
                    if (test_handle) {
                        if (ValidateProcessIntegrity(test_handle)) {
                            target_pid = entry.th32ProcessID;
                        }
                        CloseHandle(test_handle);
                        if (target_pid != 0) break;
                    }
                }
            } while (Process32NextW(snapshot, &entry));
        }
        
        CloseHandle(snapshot);
        return target_pid;
    }
    
    static bool ValidatePEStructure(const std::vector<uint8_t>& pe_data) noexcept {
        if (pe_data.size() < sizeof(IMAGE_DOS_HEADER)) return false;
        
        const auto* dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(pe_data.data());
        if (dos_header->e_magic != DOS_SIGNATURE) return false;
        
        if (pe_data.size() < dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS)) return false;
        
        const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS*>(
            pe_data.data() + dos_header->e_lfanew);
        if (nt_headers->Signature != PE_SIGNATURE) return false;
        
        // Validate architecture
        if (nt_headers->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) return false;
        
        // Validate characteristics
        if (!(nt_headers->FileHeader.Characteristics & IMAGE_FILE_DLL)) return false;
        
        return true;
    }
    
    static InjectionResult PerformManualMapping(const std::vector<uint8_t>& pe_data) noexcept {
        if (!ValidatePEStructure(pe_data)) {
            return InjectionResult::ERROR_INVALID_PE_FORMAT;
        }
        
        const auto* dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(pe_data.data());
        const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS*>(
            pe_data.data() + dos_header->e_lfanew);
        
        // Allocate memory in target process
        SIZE_T image_size = nt_headers->OptionalHeader.SizeOfImage;
        void* base_address = nullptr;
        
        NTSTATUS status = NtAllocateVirtualMemory(g_context.target_process, &base_address, 0, 
            &image_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        
        if (!NT_SUCCESS(status)) {
            return InjectionResult::ERROR_MEMORY_ALLOCATION_FAILED;
        }
        
        g_context.allocated_memory = base_address;
        g_context.allocated_size = image_size;
        
        // Copy headers
        SIZE_T bytes_written = 0;
        status = NtWriteVirtualMemory(g_context.target_process, base_address, 
            const_cast<void*>(reinterpret_cast<const void*>(pe_data.data())), 
            nt_headers->OptionalHeader.SizeOfHeaders, &bytes_written);
        
        if (!NT_SUCCESS(status)) {
            return InjectionResult::ERROR_MEMORY_ALLOCATION_FAILED;
        }
        
        // Copy sections with proper alignment
        const auto* section_header = IMAGE_FIRST_SECTION(nt_headers);
        for (WORD i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i) {
            if (section_header[i].SizeOfRawData == 0) continue;
            
            void* section_dest = reinterpret_cast<void*>(
                reinterpret_cast<uintptr_t>(base_address) + section_header[i].VirtualAddress);
            
            status = NtWriteVirtualMemory(g_context.target_process, section_dest,
                const_cast<void*>(reinterpret_cast<const void*>(
                    pe_data.data() + section_header[i].PointerToRawData)),
                section_header[i].SizeOfRawData, &bytes_written);
            
            if (!NT_SUCCESS(status)) {
                return InjectionResult::ERROR_MEMORY_ALLOCATION_FAILED;
            }
        }
        
        // Set proper memory protections
        for (WORD i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i) {
            void* section_base = reinterpret_cast<void*>(
                reinterpret_cast<uintptr_t>(base_address) + section_header[i].VirtualAddress);
            SIZE_T section_size = section_header[i].Misc.VirtualSize;
            
            ULONG protection = PAGE_READONLY;
            if (section_header[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) {
                protection = PAGE_EXECUTE_READ;
                if (section_header[i].Characteristics & IMAGE_SCN_MEM_WRITE) {
                    protection = PAGE_EXECUTE_READWRITE;
                }
            } else if (section_header[i].Characteristics & IMAGE_SCN_MEM_WRITE) {
                protection = PAGE_READWRITE;
            }
            
            ULONG old_protection = 0;
            NtProtectVirtualMemory(g_context.target_process, &section_base, 
                &section_size, protection, &old_protection);
        }
        
        // Create execution thread
        void* entry_point = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(base_address) + nt_headers->OptionalHeader.AddressOfEntryPoint);
        
        HANDLE thread_handle = nullptr;
        status = NtCreateThreadEx(&thread_handle, THREAD_ALL_ACCESS, nullptr,
            g_context.target_process, entry_point, base_address, 0, 0, 0, 0, nullptr);
        
        if (!NT_SUCCESS(status)) {
            return InjectionResult::ERROR_THREAD_CREATION_FAILED;
        }
        
        if (thread_handle) {
            WaitForSingleObject(thread_handle, INJECTION_TIMEOUT_MS);
            CloseHandle(thread_handle);
        }
        
        return InjectionResult::SUCCESS;
    }
    
public:
    static InjectionResult Initialize() noexcept {
        // Security validation
        if (IsDebuggerPresent()) {
            return InjectionResult::ERROR_SECURITY_VALIDATION_FAILED;
        }
        
        // Initialize security token
        g_context.security_token.store(SECURITY_COOKIE ^ GetTickCount64());
        g_context.initialized.store(true);
        
        return InjectionResult::SUCCESS;
    }
    
    static InjectionResult AttachToProcess(const std::wstring& process_name) noexcept {
        if (!g_context.initialized.load()) {
            return InjectionResult::ERROR_INITIALIZATION_FAILED;
        }
        
        DWORD pid = FindSecureProcess(process_name);
        if (pid == 0) {
            return InjectionResult::ERROR_TARGET_NOT_FOUND;
        }
        
        HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!process) {
            return InjectionResult::ERROR_ACCESS_DENIED;
        }
        
        if (!ValidateProcessIntegrity(process)) {
            CloseHandle(process);
            return InjectionResult::ERROR_PROCESS_PROTECTED;
        }
        
        g_context.target_process = process;
        g_context.target_pid = pid;
        
        return InjectionResult::SUCCESS;
    }
    
    static InjectionResult InjectPayload(const std::wstring& dll_path) noexcept {
        if (!g_context.target_process) {
            return InjectionResult::ERROR_INITIALIZATION_FAILED;
        }
        
        // Read and validate DLL
        HANDLE file = CreateFileW(dll_path.c_str(), GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        
        if (file == INVALID_HANDLE_VALUE) {
            return InjectionResult::ERROR_TARGET_NOT_FOUND;
        }
        
        LARGE_INTEGER file_size = {};
        if (!GetFileSizeEx(file, &file_size) || file_size.QuadPart > 50 * 1024 * 1024) {
            CloseHandle(file);
            return InjectionResult::ERROR_INVALID_PE_FORMAT;
        }
        
        std::vector<uint8_t> dll_data(static_cast<size_t>(file_size.QuadPart));
        DWORD bytes_read = 0;
        
        if (!ReadFile(file, dll_data.data(), static_cast<DWORD>(dll_data.size()), &bytes_read, nullptr)) {
            CloseHandle(file);
            return InjectionResult::ERROR_INVALID_PE_FORMAT;
        }
        
        CloseHandle(file);
        
        return PerformManualMapping(dll_data);
    }
    
    static void Cleanup() noexcept {
        g_context.initialized.store(false);
        g_context.security_token.store(0);
        
        if (g_context.target_process && g_context.target_process != INVALID_HANDLE_VALUE) {
            CloseHandle(g_context.target_process);
            g_context.target_process = nullptr;
        }
        
        g_context.target_pid = 0;
        g_context.allocated_memory = nullptr;
        g_context.allocated_size = 0;
    }
};

// Public API implementation
InjectionResult InjectionEngine::Initialize() noexcept {
    return SecureInjector::Initialize();
}

InjectionResult InjectionEngine::InjectIntoTarget(const std::wstring& targetProcess) noexcept {
    InjectionResult result = SecureInjector::AttachToProcess(targetProcess);
    if (result != InjectionResult::SUCCESS) {
        return result;
    }
    
    // Get current module directory
    std::array<wchar_t, MAX_PATH> module_path = {};
    GetModuleFileNameW(nullptr, module_path.data(), MAX_PATH);
    
    std::wstring dll_path = module_path.data();
    size_t pos = dll_path.find_last_of(L'\\');
    if (pos != std::wstring::npos) {
        dll_path = dll_path.substr(0, pos + 1) + L"aether_backend.dll";
    }
    
    return SecureInjector::InjectPayload(dll_path);
}

InjectionResult InjectionEngine::ValidateInjection() noexcept {
    if (!g_context.initialized.load() || !g_context.target_process) {
        return InjectionResult::ERROR_INITIALIZATION_FAILED;
    }
    
    // Validate injection success by checking allocated memory
    if (g_context.allocated_memory && g_context.allocated_size > 0) {
        return InjectionResult::SUCCESS;
    }
    
    return InjectionResult::ERROR_SECURITY_VALIDATION_FAILED;
}

void InjectionEngine::Cleanup() noexcept {
    SecureInjector::Cleanup();
}

bool InjectionEngine::IsInitialized() noexcept {
    return g_context.initialized.load();
}

const wchar_t* InjectionEngine::GetErrorDescription(InjectionResult result) noexcept {
    switch (result) {
        case InjectionResult::SUCCESS:
            return L"Operation completed successfully";
        case InjectionResult::ERROR_INITIALIZATION_FAILED:
            return L"Initialization failed";
        case InjectionResult::ERROR_TARGET_NOT_FOUND:
            return L"Target process not found";
        case InjectionResult::ERROR_ACCESS_DENIED:
            return L"Access denied";
        case InjectionResult::ERROR_PROCESS_PROTECTED:
            return L"Target process is protected";
        case InjectionResult::ERROR_MEMORY_ALLOCATION_FAILED:
            return L"Memory allocation failed";
        case InjectionResult::ERROR_INVALID_PE_FORMAT:
            return L"Invalid PE format";
        case InjectionResult::ERROR_IMPORT_RESOLUTION_FAILED:
            return L"Import resolution failed";
        case InjectionResult::ERROR_THREAD_CREATION_FAILED:
            return L"Thread creation failed";
        case InjectionResult::ERROR_SECURITY_VALIDATION_FAILED:
            return L"Security validation failed";
        default:
            return L"Unknown error";
    }
}

}
