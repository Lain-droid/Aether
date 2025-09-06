#include "MemoryProtection.h"
#include <intrin.h>

namespace AetherVisor {
namespace Security {

std::vector<void*> MemoryProtection::m_hiddenRegions;
bool MemoryProtection::m_initialized = false;
bool AntiDebug::m_active = false;

bool MemoryProtection::Initialize() {
    if (m_initialized) return true;
    
    EnableControlFlowGuard();
    InstallStackCanaries();
    
    m_initialized = true;
    return true;
}

void* MemoryProtection::AllocateStealthMemory(size_t size) {
    void* ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (ptr) {
        m_hiddenRegions.push_back(ptr);
        
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery(ptr, &mbi, sizeof(mbi));
        
        DWORD oldProtect;
        VirtualProtect(ptr, size, PAGE_NOACCESS, &oldProtect);
        VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &oldProtect);
        
        ObfuscateMemoryContent(ptr, size);
    }
    return ptr;
}

void MemoryProtection::FreeStealthMemory(void* ptr) {
    if (ptr) {
        auto it = std::find(m_hiddenRegions.begin(), m_hiddenRegions.end(), ptr);
        if (it != m_hiddenRegions.end()) {
            m_hiddenRegions.erase(it);
        }
        
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(ptr, &mbi, sizeof(mbi))) {
            SecureZeroMemory(ptr, mbi.RegionSize);
        }
        
        VirtualFree(ptr, 0, MEM_RELEASE);
    }
}

bool MemoryProtection::HideMemoryRegion(void* ptr, size_t size) {
    DWORD oldProtect;
    if (!VirtualProtect(ptr, size, PAGE_NOACCESS, &oldProtect)) {
        return false;
    }
    
    if (!VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }
    
    return true;
}

void MemoryProtection::ScrambleMemoryLayout() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    
    for (int i = 0; i < 10; i++) {
        void* dummy = VirtualAlloc(NULL, si.dwPageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (dummy) {
            VirtualFree(dummy, 0, MEM_RELEASE);
        }
    }
}

bool MemoryProtection::EnableControlFlowGuard() {
#ifdef _WIN64
    PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY policy = {};
    policy.EnableControlFlowGuard = 1;
    
    return SetProcessMitigationPolicy(ProcessControlFlowGuardPolicy, &policy, sizeof(policy));
#else
    return false;
#endif
}

void MemoryProtection::InstallStackCanaries() {
    DWORD canary = GetTickCount() ^ (DWORD)(uintptr_t)&canary;
    
    __try {
        DWORD* stackPtr = (DWORD*)_AddressOfReturnAddress();
        if (stackPtr && !IsBadWritePtr(stackPtr - 1, sizeof(DWORD))) {
            *(stackPtr - 1) = canary;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool MemoryProtection::ValidateReturnAddresses() {
    __try {
        void* returnAddr = _ReturnAddress();
        if (!returnAddr) return false;
        
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(returnAddr, &mbi, sizeof(mbi))) {
            return false;
        }
        
        if (mbi.State != MEM_COMMIT || !(mbi.Protect & PAGE_EXECUTE_READ)) {
            return false;
        }
        
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool MemoryProtection::DetectMemoryScanning() {
    static DWORD lastScanTime = 0;
    DWORD currentTime = GetTickCount();
    
    if (currentTime - lastScanTime < 100) {
        return true;
    }
    
    lastScanTime = currentTime;
    
    MEMORY_BASIC_INFORMATION mbi;
    void* address = NULL;
    
    while (VirtualQuery(address, &mbi, sizeof(mbi))) {
        if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE) {
            if (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) {
                DWORD* ptr = (DWORD*)mbi.BaseAddress;
                if (!IsBadReadPtr(ptr, sizeof(DWORD))) {
                    if (*ptr == 0xDEADBEEF) {
                        return true;
                    }
                }
            }
        }
        address = (BYTE*)mbi.BaseAddress + mbi.RegionSize;
    }
    
    return false;
}

void MemoryProtection::ObfuscateMemoryContent(void* ptr, size_t size) {
    if (!ptr || size == 0) return;
    
    DWORD key = GetTickCount() ^ (DWORD)(uintptr_t)ptr;
    BYTE* bytes = (BYTE*)ptr;
    
    for (size_t i = 0; i < size; i++) {
        bytes[i] ^= (BYTE)(key >> (i % 4 * 8));
    }
}

bool AntiDebug::Initialize() {
    if (m_active) return true;
    
    InstallAntiDebugHooks();
    BlockDebuggerAttachment();
    
    m_active = true;
    return true;
}

bool AntiDebug::IsDebuggerPresent() {
    if (::IsDebuggerPresent()) return true;
    
    BOOL debuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &debuggerPresent);
    if (debuggerPresent) return true;
    
    HANDLE debugObject = NULL;
    NTSTATUS status = NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugObjectHandle, &debugObject, sizeof(debugObject), NULL);
    if (NT_SUCCESS(status) && debugObject) return true;
    
    return false;
}

bool AntiDebug::DetectRemoteDebugger() {
    BOOL debuggerPresent = FALSE;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &debuggerPresent) && debuggerPresent) {
        return true;
    }
    
    return false;
}

bool AntiDebug::CheckDebuggerTiming() {
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    
    __nop();
    __nop();
    __nop();
    
    QueryPerformanceCounter(&end);
    
    DWORD elapsed = (DWORD)((end.QuadPart - start.QuadPart) * 1000000 / freq.QuadPart);
    
    return elapsed > 1000;
}

bool AntiDebug::DetectHardwareBreakpoints() {
    CONTEXT ctx = {};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    
    if (GetThreadContext(GetCurrentThread(), &ctx)) {
        if (ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3) {
            return true;
        }
    }
    
    return false;
}

void AntiDebug::InstallAntiDebugHooks() {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return;
    
    FARPROC dbgBreakPoint = GetProcAddress(ntdll, "DbgBreakPoint");
    if (dbgBreakPoint) {
        DWORD oldProtect;
        if (VirtualProtect(dbgBreakPoint, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *(BYTE*)dbgBreakPoint = 0xC3;
            VirtualProtect(dbgBreakPoint, 1, oldProtect, &oldProtect);
        }
    }
}

void AntiDebug::BlockDebuggerAttachment() {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return;
    
    FARPROC dbgUiRemoteBreakin = GetProcAddress(ntdll, "DbgUiRemoteBreakin");
    if (dbgUiRemoteBreakin) {
        DWORD oldProtect;
        if (VirtualProtect(dbgUiRemoteBreakin, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *(BYTE*)dbgUiRemoteBreakin = 0xC3;
            VirtualProtect(dbgUiRemoteBreakin, 1, oldProtect, &oldProtect);
        }
    }
}

}
}
