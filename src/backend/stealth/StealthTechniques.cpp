#include "StealthTechniques.h"
#include <TlHelp32.h>
#include <intrin.h>

namespace AetherVisor {
namespace Security {

bool ProcessStealth::m_active = false;
DWORD NetworkStealth::m_obfuscationKey = 0;
std::vector<void*> CodeObfuscation::m_mutatedRegions;

bool ProcessStealth::Initialize() {
    if (m_active) return true;
    
    __try {
        SpoofPEB();
        HideThreads();
        m_active = true;
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool ProcessStealth::SpoofPEB() {
    __try {
#ifdef _WIN64
        PPEB peb = (PPEB)__readgsqword(0x60);
#else
        PPEB peb = (PPEB)__readfsdword(0x30);
#endif
        if (!peb || IsBadWritePtr(peb, sizeof(PEB))) return false;
        
        peb->BeingDebugged = FALSE;
        
        PPEB_LDR_DATA ldr = peb->Ldr;
        if (ldr && !IsBadWritePtr(ldr, sizeof(PEB_LDR_DATA))) {
            PLIST_ENTRY head = &ldr->InMemoryOrderModuleList;
            PLIST_ENTRY current = head->Flink;
            
            while (current && current != head && !IsBadReadPtr(current, sizeof(LIST_ENTRY))) {
                PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(current, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
                
                if (entry && !IsBadReadPtr(entry, sizeof(LDR_DATA_TABLE_ENTRY))) {
                    if (entry->FullDllName.Buffer && !IsBadReadPtr(entry->FullDllName.Buffer, entry->FullDllName.Length)) {
                        std::wstring dllName = entry->FullDllName.Buffer;
                        if (dllName.find(L"aether") != std::wstring::npos) {
                            entry->FullDllName.Length = 0;
                            entry->FullDllName.MaximumLength = 0;
                            entry->FullDllName.Buffer = nullptr;
                        }
                    }
                }
                
                current = current->Flink;
            }
        }
        
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool ProcessStealth::HideThreads() {
    __try {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        
        THREADENTRY32 te32 = {};
        te32.dwSize = sizeof(THREADENTRY32);
        
        if (Thread32First(snapshot, &te32)) {
            do {
                if (te32.th32OwnerProcessID == GetCurrentProcessId()) {
                    HANDLE thread = OpenThread(THREAD_SET_INFORMATION, FALSE, te32.th32ThreadID);
                    if (thread) {
                        SetThreadPriority(thread, THREAD_PRIORITY_IDLE);
                        CloseHandle(thread);
                    }
                }
            } while (Thread32Next(snapshot, &te32));
        }
        
        CloseHandle(snapshot);
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool ProcessStealth::MaskProcessMemory() {
    __try {
        MEMORY_BASIC_INFORMATION mbi = {};
        void* address = NULL;
        
        while (VirtualQuery(address, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE) {
                if (mbi.Protect & PAGE_EXECUTE_READWRITE) {
                    DWORD oldProtect;
                    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READONLY, &oldProtect);
                    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, oldProtect, &oldProtect);
                }
            }
            address = (BYTE*)mbi.BaseAddress + mbi.RegionSize;
        }
        
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool NetworkStealth::Initialize() {
    __try {
        m_obfuscationKey = GetTickCount() ^ 0xDEADBEEF;
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void NetworkStealth::ObfuscateTraffic(std::vector<BYTE>& data) {
    __try {
        if (data.empty()) return;
        
        for (size_t i = 0; i < data.size(); i++) {
            data[i] ^= (BYTE)(m_obfuscationKey >> (i % 4 * 8));
            data[i] = (data[i] << 3) | (data[i] >> 5);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

void NetworkStealth::DeobfuscateTraffic(std::vector<BYTE>& data) {
    __try {
        if (data.empty()) return;
        
        for (size_t i = 0; i < data.size(); i++) {
            data[i] = (data[i] >> 3) | (data[i] << 5);
            data[i] ^= (BYTE)(m_obfuscationKey >> (i % 4 * 8));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool NetworkStealth::DetectNetworkMonitoring() {
    __try {
        HMODULE ws2_32 = GetModuleHandleA("ws2_32.dll");
        if (!ws2_32) return false;
        
        FARPROC sendFunc = GetProcAddress(ws2_32, "send");
        FARPROC recvFunc = GetProcAddress(ws2_32, "recv");
        
        if (!sendFunc || !recvFunc) return false;
        
        BYTE* sendBytes = (BYTE*)sendFunc;
        BYTE* recvBytes = (BYTE*)recvFunc;
        
        if (!IsBadReadPtr(sendBytes, 2) && (sendBytes[0] == 0xE9 || sendBytes[0] == 0xE8)) return true;
        if (!IsBadReadPtr(recvBytes, 2) && (recvBytes[0] == 0xE9 || recvBytes[0] == 0xE8)) return true;
        
        return false;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void NetworkStealth::GenerateRealisticTraffic() {
    __try {
        WSADATA wsaData = {};
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;
        
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock != INVALID_SOCKET) {
            sockaddr_in addr = {};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr("8.8.8.8");
            addr.sin_port = htons(53);
            
            if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
                const char* dummyData = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
                send(sock, dummyData, (int)strlen(dummyData), 0);
            }
            
            closesocket(sock);
        }
        
        WSACleanup();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool CodeObfuscation::Initialize() {
    return true;
}

void CodeObfuscation::MutateCodeAtRuntime(void* codePtr, size_t size) {
    __try {
        if (!codePtr || size == 0 || IsBadWritePtr(codePtr, size)) return;
        
        DWORD oldProtect;
        if (!VirtualProtect(codePtr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) return;
        
        BYTE* code = (BYTE*)codePtr;
        
        for (size_t i = 0; i < size - 1; i++) {
            if (code[i] == 0x90) {
                code[i] = 0x40;
                if (i + 1 < size) code[i + 1] = 0x90;
            }
            
            if (code[i] == 0xCC) {
                code[i] = 0x90;
            }
        }
        
        VirtualProtect(codePtr, size, oldProtect, &oldProtect);
        m_mutatedRegions.push_back(codePtr);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

void* CodeObfuscation::CreatePolymorphicCode(void* originalCode, size_t size) {
    __try {
        if (!originalCode || size == 0 || IsBadReadPtr(originalCode, size)) return nullptr;
        
        void* polymorphicCode = VirtualAlloc(NULL, size * 2, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!polymorphicCode) return nullptr;
        
        BYTE* src = (BYTE*)originalCode;
        BYTE* dst = (BYTE*)polymorphicCode;
        
        for (size_t i = 0; i < size; i++) {
            *dst++ = 0x90;
            *dst++ = src[i];
        }
        
        return polymorphicCode;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

void CodeObfuscation::DestroyPolymorphicCode(void* polymorphicCode) {
    __try {
        if (polymorphicCode) {
            VirtualFree(polymorphicCode, 0, MEM_RELEASE);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool EnvironmentDetection::Initialize() {
    return true;
}

bool EnvironmentDetection::DetectVirtualMachine() {
    __try {
        if (CheckCPUID()) return true;
        if (CheckRegistryKeys()) return true;
        if (CheckRunningProcesses()) return true;
        
        return false;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return true;
    }
}

bool EnvironmentDetection::CheckCPUID() {
    __try {
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 0);
        
        char vendor[13] = {};
        memcpy(vendor, &cpuInfo[1], 4);
        memcpy(vendor + 4, &cpuInfo[3], 4);
        memcpy(vendor + 8, &cpuInfo[2], 4);
        
        if (strstr(vendor, "VMware") || strstr(vendor, "VBox") || strstr(vendor, "QEMU")) {
            return true;
        }
        
        return false;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return true;
    }
}

bool EnvironmentDetection::CheckRegistryKeys() {
    __try {
        HKEY hKey;
        const char* vmKeys[] = {
            "SYSTEM\\CurrentControlSet\\Enum\\SCSI\\Disk&Ven_VMware_",
            "SOFTWARE\\VMware, Inc.\\VMware Tools",
            "SOFTWARE\\Oracle\\VirtualBox Guest Additions"
        };
        
        for (int i = 0; i < 3; i++) {
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, vmKeys[i], 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return true;
            }
        }
        
        return false;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return true;
    }
}

bool EnvironmentDetection::CheckRunningProcesses() {
    __try {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        
        PROCESSENTRY32 pe32 = {};
        pe32.dwSize = sizeof(PROCESSENTRY32);
        
        const char* vmProcesses[] = {
            "vmware.exe", "vmtoolsd.exe", "vboxservice.exe", "vboxtray.exe"
        };
        
        if (Process32First(snapshot, &pe32)) {
            do {
                for (int i = 0; i < 4; i++) {
                    if (_stricmp(pe32.szExeFile, vmProcesses[i]) == 0) {
                        CloseHandle(snapshot);
                        return true;
                    }
                }
            } while (Process32Next(snapshot, &pe32));
        }
        
        CloseHandle(snapshot);
        return false;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return true;
    }
}

bool EnvironmentDetection::DetectAnalysisTools() {
    __try {
        const char* analysisTools[] = {
            "ollydbg.exe", "x64dbg.exe", "ida.exe", "ida64.exe", 
            "wireshark.exe", "procmon.exe", "processhacker.exe"
        };
        
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        
        PROCESSENTRY32 pe32 = {};
        pe32.dwSize = sizeof(PROCESSENTRY32);
        
        if (Process32First(snapshot, &pe32)) {
            do {
                for (int i = 0; i < 7; i++) {
                    if (_stricmp(pe32.szExeFile, analysisTools[i]) == 0) {
                        CloseHandle(snapshot);
                        return true;
                    }
                }
            } while (Process32Next(snapshot, &pe32));
        }
        
        CloseHandle(snapshot);
        return false;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return true;
    }
}

}
}
