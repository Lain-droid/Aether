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
        INTEGRITY_VIOLATION
    };
    
    struct SecurityEvent {
        SecurityEventType type;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        uint32_t severity;
        
        SecurityEvent(SecurityEventType t, const std::string& msg, uint32_t sev = 5)
            : type(t), message(msg), timestamp(std::chrono::system_clock::now()), severity(sev) {}
    };
    
    struct SecurityConfig {
        bool enable_stack_guard = true;
        bool enable_heap_protection = true;
        bool enable_anti_debug = true;
        bool enable_integrity_checks = true;
        uint32_t max_memory_mb = 512;
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
