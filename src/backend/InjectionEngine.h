#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <memory>

namespace Aether::Core {

enum class InjectionResult : uint32_t {
    SUCCESS = 0x00000000,
    ERROR_INITIALIZATION_FAILED = 0x80000001,
    ERROR_TARGET_NOT_FOUND = 0x80000002,
    ERROR_ACCESS_DENIED = 0x80000003,
    ERROR_PROCESS_PROTECTED = 0x80000004,
    ERROR_MEMORY_ALLOCATION_FAILED = 0x80000005,
    ERROR_INVALID_PE_FORMAT = 0x80000006,
    ERROR_IMPORT_RESOLUTION_FAILED = 0x80000007,
    ERROR_THREAD_CREATION_FAILED = 0x80000008,
    ERROR_SECURITY_VALIDATION_FAILED = 0x80000009
};

class InjectionEngine {
private:
    static constexpr uint32_t SECURITY_MAGIC = 0xDEADBEEF;
    static constexpr size_t MAX_INJECTION_ATTEMPTS = 3;
    static constexpr uint32_t INJECTION_TIMEOUT_MS = 30000;
    
    InjectionEngine() = delete;
    InjectionEngine(const InjectionEngine&) = delete;
    InjectionEngine& operator=(const InjectionEngine&) = delete;

public:
    static InjectionResult Initialize() noexcept;
    static InjectionResult InjectIntoTarget(const std::wstring& targetProcess) noexcept;
    static InjectionResult ValidateInjection() noexcept;
    static void Cleanup() noexcept;
    
    static bool IsInitialized() noexcept;
    static const wchar_t* GetErrorDescription(InjectionResult result) noexcept;
};

}
