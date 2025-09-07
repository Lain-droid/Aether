#pragma once
#include <windows.h>
#include <string>

namespace Aether {

enum class InjectionResult : uint32_t {
    SUCCESS = 0x00000000,
    ERROR_INITIALIZATION_FAILED = 0x80000001,
    ERROR_TARGET_NOT_FOUND = 0x80000002,
    ERROR_ACCESS_DENIED = 0x80000003,
    ERROR_MEMORY_ALLOCATION_FAILED = 0x80000005,
    ERROR_THREAD_CREATION_FAILED = 0x80000008
};

class InjectionEngine {
public:
    static InjectionResult Initialize();
    static InjectionResult InjectIntoTarget(const std::wstring& targetProcess);
    static void Cleanup();
    static bool IsInitialized();
};

}
