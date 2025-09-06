#include "SyscallEvasion.h"
#include <intrin.h>

namespace AetherVisor {
namespace Security {

std::unordered_map<DWORD, SyscallEntry> SyscallEvasion::m_syscallTable;
bool SyscallEvasion::m_initialized = false;
bool AntiCheatEvasion::m_active = false;

bool SyscallEvasion::Initialize() {
    if (m_initialized) return true;
    
    if (!BuildSyscallTable()) return false;
    
    m_initialized = true;
    return true;
}

NTSTATUS SyscallEvasion::HellsGate(DWORD hash, PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4) {
    if (!m_initialized) return STATUS_UNSUCCESSFUL;
    
    auto it = m_syscallTable.find(hash);
    if (it == m_syscallTable.end()) return STATUS_NOT_FOUND;
    
    DWORD syscallNumber = it->second.number;
    
#ifdef _WIN64
    return ((NTSTATUS(*)(PVOID, PVOID, PVOID, PVOID))([syscallNumber](PVOID a1, PVOID a2, PVOID a3, PVOID a4) -> NTSTATUS {
        __asm {
            mov eax, syscallNumber
            mov r10, rcx
            syscall
        }
        return STATUS_SUCCESS;
    }))(arg1, arg2, arg3, arg4);
#else
    return STATUS_NOT_SUPPORTED;
#endif
}

NTSTATUS SyscallEvasion::HalosGate(DWORD hash, PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4) {
    if (!m_initialized) return STATUS_UNSUCCESSFUL;
    
    auto it = m_syscallTable.find(hash);
    if (it == m_syscallTable.end()) return STATUS_NOT_FOUND;
    
    if (it->second.isHooked) {
        DWORD cleanSyscall = FindNearbyCleanSyscall(it->second.address);
        if (cleanSyscall == 0) return STATUS_UNSUCCESSFUL;
        
#ifdef _WIN64
        return ((NTSTATUS(*)(PVOID, PVOID, PVOID, PVOID))([cleanSyscall](PVOID a1, PVOID a2, PVOID a3, PVOID a4) -> NTSTATUS {
            __asm {
                mov eax, cleanSyscall
                mov r10, rcx
                syscall
            }
            return STATUS_SUCCESS;
        }))(arg1, arg2, arg3, arg4);
#else
        return STATUS_NOT_SUPPORTED;
#endif
    } else {
        return HellsGate(hash, arg1, arg2, arg3, arg4);
    }
}

bool SyscallEvasion::IsHooked(PVOID function) {
    BYTE* func = (BYTE*)function;
    
    if (func[0] == 0xE9 || func[0] == 0xE8) return true;
    if (func[0] == 0xFF && func[1] == 0x25) return true;
    if (func[0] == 0x48 && func[1] == 0xB8) return true;
    
    return false;
}

DWORD SyscallEvasion::HashFunction(const char* str) {
    DWORD hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + *str++;
    }
    return hash;
}

bool SyscallEvasion::BuildSyscallTable() {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return false;
    
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)ntdll;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)ntdll + dosHeader->e_lfanew);
    PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)ntdll + 
        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    
    DWORD* names = (DWORD*)((BYTE*)ntdll + exportDir->AddressOfNames);
    WORD* ordinals = (WORD*)((BYTE*)ntdll + exportDir->AddressOfNameOrdinals);
    DWORD* functions = (DWORD*)((BYTE*)ntdll + exportDir->AddressOfFunctions);
    
    for (DWORD i = 0; i < exportDir->NumberOfNames; i++) {
        char* name = (char*)((BYTE*)ntdll + names[i]);
        if (strncmp(name, "Nt", 2) == 0 || strncmp(name, "Zw", 2) == 0) {
            PVOID funcAddr = (BYTE*)ntdll + functions[ordinals[i]];
            
            if (funcAddr && !IsBadReadPtr(funcAddr, 8)) {
                DWORD syscallNum = *(DWORD*)((BYTE*)funcAddr + 4);
                
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

DWORD SyscallEvasion::FindNearbyCleanSyscall(PVOID hookedFunc) {
    BYTE* base = (BYTE*)hookedFunc;
    
    for (int offset = -0x20; offset <= 0x20; offset += 0x10) {
        BYTE* candidate = base + offset;
        if (!IsBadReadPtr(candidate, 8) && !IsHooked(candidate)) {
            if (candidate[0] == 0x4C && candidate[1] == 0x8B && candidate[2] == 0xD1) {
                return *(DWORD*)(candidate + 4);
            }
        }
    }
    
    return 0;
}

bool AntiCheatEvasion::Initialize() {
    if (m_active) return true;
    
    DisableETW();
    BypassAMSI();
    
    m_active = true;
    return true;
}

void AntiCheatEvasion::DisableETW() {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return;
    
    FARPROC etwEventWrite = GetProcAddress(ntdll, "EtwEventWrite");
    if (etwEventWrite) {
        DWORD oldProtect;
        if (VirtualProtect(etwEventWrite, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            *(BYTE*)etwEventWrite = 0xC3;
            VirtualProtect(etwEventWrite, 1, oldProtect, &oldProtect);
        }
    }
}

void AntiCheatEvasion::BypassAMSI() {
    HMODULE amsi = LoadLibraryA("amsi.dll");
    if (!amsi) return;
    
    FARPROC amsiScanBuffer = GetProcAddress(amsi, "AmsiScanBuffer");
    if (amsiScanBuffer) {
        DWORD oldProtect;
        if (VirtualProtect(amsiScanBuffer, 6, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(amsiScanBuffer, "\x48\x31\xC0\x48\xFF\xC0\xC3", 7);
            VirtualProtect(amsiScanBuffer, 6, oldProtect, &oldProtect);
        }
    }
}

bool AntiCheatEvasion::DetectHyperion() {
    HMODULE roblox = GetModuleHandleA("RobloxPlayerBeta.exe");
    if (!roblox) return false;
    
    MODULEINFO modInfo;
    if (!GetModuleInformation(GetCurrentProcess(), roblox, &modInfo, sizeof(modInfo))) {
        return false;
    }
    
    BYTE* base = (BYTE*)modInfo.lpBaseOfDll;
    SIZE_T size = modInfo.SizeOfImage;
    
    const BYTE signature[] = {0x48, 0x8B, 0x05};
    
    for (SIZE_T i = 0; i < size - sizeof(signature); i++) {
        if (memcmp(base + i, signature, sizeof(signature)) == 0) {
            return true;
        }
    }
    
    return false;
}

void AntiCheatEvasion::RandomizeTimings() {
    DWORD randomDelay = (GetTickCount() % 50) + 10;
    Sleep(randomDelay);
}

}
}
