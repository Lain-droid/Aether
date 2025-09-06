#pragma once
#include <Windows.h>
#include <winternl.h>
#include <vector>
#include <unordered_map>

namespace AetherVisor {
namespace Security {

struct SyscallEntry {
    DWORD number;
    PVOID address;
    bool isHooked;
};

class SyscallEvasion {
public:
    static bool Initialize();
    static void Cleanup();
    
    static NTSTATUS HellsGate(DWORD hash, PVOID arg1 = nullptr, PVOID arg2 = nullptr, PVOID arg3 = nullptr, PVOID arg4 = nullptr);
    static NTSTATUS HalosGate(DWORD hash, PVOID arg1 = nullptr, PVOID arg2 = nullptr, PVOID arg3 = nullptr, PVOID arg4 = nullptr);
    
    static bool IsHooked(PVOID function);
    static DWORD GetSyscallNumber(DWORD hash);

private:
    static bool BuildSyscallTable();
    static DWORD HashFunction(const char* str);
    static DWORD FindNearbyCleanSyscall(PVOID hookedFunc);
    
    static std::unordered_map<DWORD, SyscallEntry> m_syscallTable;
    static bool m_initialized;
};

class AntiCheatEvasion {
public:
    static bool Initialize();
    
    static bool DetectHyperion();
    static void DisableETW();
    static void BypassAMSI();
    static void RandomizeTimings();

private:
    static bool m_active;
};

}
}
