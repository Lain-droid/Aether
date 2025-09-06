#pragma once
#include "syscall/SyscallEvasion.h"
#include "memory/MemoryProtection.h"
#include "stealth/StealthTechniques.h"
#include "antidetect/AntiDetection.h"
#include <atomic>
#include <thread>

namespace AetherVisor {
namespace Security {

class SecurityManager {
public:
    static SecurityManager& GetInstance();
    
    bool InitializeAllSecurity();
    void StartSecurityMonitoring();
    void StopSecurityMonitoring();
    
    bool IsSecurityActive() const;
    bool IsCompromised() const;
    void EmergencyShutdown();
    
    void RunPeriodicChecks();

private:
    SecurityManager() = default;
    ~SecurityManager() = default;
    
    void SecurityMonitoringThread();
    void PerformSecurityChecks();
    
    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_monitoring{false};
    std::atomic<bool> m_compromised{false};
    std::thread m_monitorThread;
};

class SecurityConfig {
public:
    static bool LoadConfig();
    static void SaveConfig();
    
    static bool IsSyscallEvasionEnabled();
    static bool IsMemoryProtectionEnabled();
    static bool IsStealthModeEnabled();
    static bool IsAntiDetectionEnabled();
    
    static void EnableSyscallEvasion(bool enable);
    static void EnableMemoryProtection(bool enable);
    static void EnableStealthMode(bool enable);
    static void EnableAntiDetection(bool enable);

private:
    static bool m_syscallEvasion;
    static bool m_memoryProtection;
    static bool m_stealthMode;
    static bool m_antiDetection;
};

}
}
