#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#include <winternl.h>
#include <psapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Forward declarations for PEB structures if not already defined
#ifndef _PEB_LDR_DATA_DEFINED
#define _PEB_LDR_DATA_DEFINED
typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;
#endif

#ifndef _LDR_DATA_TABLE_ENTRY_DEFINED  
#define _LDR_DATA_TABLE_ENTRY_DEFINED
typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
#endif

using ByteType = BYTE;
#else
using ByteType = unsigned char;
#endif

namespace AetherVisor::Security {
    enum class SecurityEventType {
        DEBUGGER_DETECTED,
        MEMORY_CORRUPTION,
        INJECTION_ATTEMPT,
        RESOURCE_LIMIT_EXCEEDED,
        INTEGRITY_VIOLATION,
        BUFFER_OVERFLOW_ATTEMPT,
        VM_DETECTED,
        SUSPICIOUS_ACTIVITY
    };
    
    struct SecurityEvent {
        SecurityEventType type;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        uint32_t severity;
        
        // Additional fields for compatibility
        std::string description;
        void* memory_address = nullptr;
        size_t data_size = 0;
        uint32_t thread_id = 0;
        uint32_t process_id = 0;
        
        SecurityEvent(SecurityEventType t, const std::string& msg, uint32_t sev = 5)
            : type(t), message(msg), timestamp(std::chrono::system_clock::now()), severity(sev), 
              description(msg), thread_id(GetCurrentThreadId()), process_id(GetCurrentProcessId()) {}
    };
    
    struct SecurityConfig {
        bool enable_stack_guard = true;
        bool enable_heap_protection = true;
        bool enable_anti_debug = true;
        bool enable_integrity_checks = true;
        bool enable_aslr = true;
        bool enable_dep = true;
        bool enable_control_flow_guard = true;
        bool enable_safe_seh = true;
        bool enable_memory_sanitizer = true;
        bool enable_address_sanitizer = true;
        bool enable_anti_vm = true;
        uint32_t max_memory_mb = 512;
        uint32_t max_string_length = 4096;
        uint32_t max_array_size = 1048576;
        uint32_t max_recursion_depth = 1000;
        uint32_t max_memory_allocation = 1073741824;
        uint32_t max_execution_time = 30000;
    };
    
    struct OptimizationStats {
        size_t instructions_eliminated = 0;
        size_t bytes_saved = 0;
        double optimization_time_ms = 0.0;
    };
    
    enum class OptimizationLevel {
        NONE,
        BASIC,
        MEDIUM,
        AGGRESSIVE
    };
}
