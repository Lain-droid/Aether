#pragma once
#include <Windows.h>
#include <vector>

namespace AetherVisor {
namespace Security {

class MemoryProtection {
public:
    static bool Initialize();
    
    static void* AllocateStealthMemory(size_t size);
    static void FreeStealthMemory(void* ptr);
    static bool HideMemoryRegion(void* ptr, size_t size);
    static void ScrambleMemoryLayout();
    
    static bool EnableControlFlowGuard();
    static void InstallStackCanaries();
    static bool ValidateReturnAddresses();
    
    static bool DetectMemoryScanning();
    static void ObfuscateMemoryContent(void* ptr, size_t size);

private:
    static std::vector<void*> m_hiddenRegions;
    static bool m_initialized;
};

class AntiDebug {
public:
    static bool Initialize();
    
    static bool IsDebuggerPresent();
    static bool DetectRemoteDebugger();
    static bool CheckDebuggerTiming();
    static bool DetectHardwareBreakpoints();
    
    static void InstallAntiDebugHooks();
    static void BlockDebuggerAttachment();

private:
    static bool m_active;
};

}
}
