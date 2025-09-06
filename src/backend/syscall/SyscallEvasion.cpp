#include "SyscallEvasion.h"
#include <intrin.h>

namespace AetherVisor {
namespace Security {

std::unordered_map<DWORD, SyscallEntry> SyscallEvasion::m_syscallTable;
bool SyscallEvasion::m_initialized = false;
bool AntiCheatEvasion::m_active = false;

bool SyscallEvasion::Initialize() {
    if (m_initialized) return true;
    
    __try {
        if (!BuildSyscallTable()) return false;
        m_initialized = true;
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

NTSTATUS SyscallEvasion::HellsGate(DWORD hash, PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4) {
    if (!m_initialized) return 0xC0000001L;
    
    auto it = m_syscallTable.find(hash);
    if (it == m_syscallTable.end()) return 0xC0000225L;
    
    DWORD syscallNumber = it->second.number;
    
#ifdef _WIN64
    __try {
        // Use external assembly function for x64
        extern "C" NTSTATUS ExecuteSyscall(DWORD syscallNumber, void* args);
        return ExecuteSyscall(syscallNumber, args);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return 0xC0000001L;
    }
#else
    return 0xC0000001L;
#endif
}

NTSTATUS SyscallEvasion::HalosGate(DWORD hash, PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4) {
    if (!m_initialized) return 0xC0000001L;
    
    auto it = m_syscallTable.find(hash);
    if (it == m_syscallTable.end()) return 0xC0000225L;
    
    if (it->second.isHooked) {
        DWORD cleanSyscall = FindNearbyCleanSyscall(it->second.address);
        if (cleanSyscall == 0) return 0xC0000001L;
        
#ifdef _WIN64
        __try {
            NTSTATUS result;
            __asm {
                // Use external assembly function for x64
                extern "C" NTSTATUS ExecuteSyscall(DWORD syscallNumber, void* args);
                return ExecuteSyscall(cleanSyscall, args);
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                return 0xC0000001L;
            }
#endif
    }
    
    return HellsGate(hash, arg1, arg2, arg3, arg4);
}

bool SyscallEvasion::IsHooked(PVOID function) {
    __try {
        if (!function || IsBadReadPtr(function, 8)) return false;
        
        BYTE* func = (BYTE*)function;
        
        if (func[0] == 0xE9 || func[0] == 0xE8) return true;
        if (func[0] == 0xFF && func[1] == 0x25) return true;
        if (func[0] == 0x48 && func[1] == 0xB8) return true;
        if (func[0] == 0x68) return true;
        
        return false;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return true;
    }
}

DWORD SyscallEvasion::HashFunction(const char* str) {
    if (!str) return 0;
    
    DWORD hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + (DWORD)*str++;
    }
    return hash;
}

bool SyscallEvasion::BuildSyscallTable() {
    __try {
        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (!ntdll) return false;
        
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)ntdll;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;
        
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)ntdll + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return false;
        
        PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)ntdll + 
            ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        
        if (!exportDir) return false;
        
        DWORD* names = (DWORD*)((BYTE*)ntdll + exportDir->AddressOfNames);
        WORD* ordinals = (WORD*)((BYTE*)ntdll + exportDir->AddressOfNameOrdinals);
        DWORD* functions = (DWORD*)((BYTE*)ntdll + exportDir->AddressOfFunctions);
        
        for (DWORD i = 0; i < exportDir->NumberOfNames; i++) {
            char* name = (char*)((BYTE*)ntdll + names[i]);
            if (!name || IsBadStringPtrA(name, 32)) continue;
            
            if ((name[0] == 'N' && name[1] == 't') || (name[0] == 'Z' && name[1] == 'w')) {
                PVOID funcAddr = (BYTE*)ntdll + functions[ordinals[i]];
                if (!funcAddr || IsBadReadPtr(funcAddr, 8)) continue;
                
                BYTE* funcBytes = (BYTE*)funcAddr;
                if (funcBytes[0] == 0x4C && funcBytes[1] == 0x8B && funcBytes[2] == 0xD1) {
                    DWORD syscallNum = *(DWORD*)(funcBytes + 4);
                    
                    SyscallEntry entry;
                    entry.number = syscallNum;
                    entry.address = funcAddr;
                    entry.isHooked = IsHooked(funcAddr);
                    
                    m_syscallTable[HashFunction(name)] = entry;
                }
            }
        }
        
        return !m_syscallTable.empty();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

DWORD SyscallEvasion::FindNearbyCleanSyscall(PVOID hookedFunc) {
    __try {
        BYTE* base = (BYTE*)hookedFunc;
        
        for (int offset = -0x20; offset <= 0x20; offset += 0x10) {
            BYTE* candidate = base + offset;
            if (IsBadReadPtr(candidate, 8)) continue;
            
            if (!IsHooked(candidate)) {
                if (candidate[0] == 0x4C && candidate[1] == 0x8B && candidate[2] == 0xD1) {
                    return *(DWORD*)(candidate + 4);
                }
            }
        }
        
        return 0;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

bool AntiCheatEvasion::Initialize() {
    if (m_active) return true;
    
    __try {
        DisableETW();
        BypassAMSI();
        m_active = true;
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void AntiCheatEvasion::DisableETW() {
    __try {
        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (!ntdll) return;
        
        FARPROC etwEventWrite = GetProcAddress(ntdll, "EtwEventWrite");
        if (etwEventWrite && !IsBadWritePtr(etwEventWrite, 1)) {
            DWORD oldProtect;
            if (VirtualProtect(etwEventWrite, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                *(BYTE*)etwEventWrite = 0xC3;
                VirtualProtect(etwEventWrite, 1, oldProtect, &oldProtect);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

void AntiCheatEvasion::BypassAMSI() {
    __try {
        HMODULE amsi = LoadLibraryA("amsi.dll");
        if (!amsi) return;
        
        FARPROC amsiScanBuffer = GetProcAddress(amsi, "AmsiScanBuffer");
        if (amsiScanBuffer && !IsBadWritePtr(amsiScanBuffer, 6)) {
            DWORD oldProtect;
            if (VirtualProtect(amsiScanBuffer, 6, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                BYTE patch[] = {0x48, 0x31, 0xC0, 0x48, 0xFF, 0xC0, 0xC3};
                memcpy(amsiScanBuffer, patch, sizeof(patch));
                VirtualProtect(amsiScanBuffer, 6, oldProtect, &oldProtect);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool AntiCheatEvasion::DetectHyperion() {
    __try {
        HMODULE roblox = GetModuleHandleA("RobloxPlayerBeta.exe");
        if (!roblox) return false;
        
        MODULEINFO modInfo = {};
        if (!GetModuleInformation(GetCurrentProcess(), roblox, &modInfo, sizeof(modInfo))) {
            return false;
        }
        
        BYTE* base = (BYTE*)modInfo.lpBaseOfDll;
        SIZE_T size = modInfo.SizeOfImage;
        
        if (!base || IsBadReadPtr(base, size)) return false;
        
        const BYTE signature[] = {0x48, 0x8B, 0x05};
        
        for (SIZE_T i = 0; i < size - sizeof(signature); i++) {
            if (!IsBadReadPtr(base + i, sizeof(signature))) {
                if (memcmp(base + i, signature, sizeof(signature)) == 0) {
                    return true;
                }
            }
        }
        
        return false;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void AntiCheatEvasion::RandomizeTimings() {
    __try {
        DWORD randomDelay = (GetTickCount() % 50) + 10;
        Sleep(randomDelay);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

}
}
