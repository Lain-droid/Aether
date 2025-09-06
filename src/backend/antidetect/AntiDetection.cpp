#include "AntiDetection.h"
#include "../security/SecurityTypes.h"
#include <random>
#include <chrono>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

namespace AetherVisor {
namespace AntiDetect {

std::vector<BYTE> SignatureEvasion::m_originalSignatures;
std::vector<BYTE> SignatureEvasion::m_mutatedSignatures;

bool HyperionEvasion::Initialize() {
    __try {
        if (DetectHyperion()) {
            BypassHyperionChecks();
            SpoofRobloxMetrics();
        }
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool HyperionEvasion::DetectHyperion() {
    __try {
        HMODULE roblox = GetModuleHandleA("RobloxPlayerBeta.exe");
        if (!roblox) return false;
        
        return ScanForHyperionSignatures();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool HyperionEvasion::ScanForHyperionSignatures() {
    __try {
        MODULEINFO modInfo = {};
        HMODULE roblox = GetModuleHandleA("RobloxPlayerBeta.exe");
        if (!GetModuleInformation(GetCurrentProcess(), roblox, &modInfo, sizeof(modInfo))) {
            return false;
        }
        
        BYTE* base = (BYTE*)modInfo.lpBaseOfDll;
        SIZE_T size = modInfo.SizeOfImage;
        
        if (!base || IsBadReadPtr(base, size)) return false;
        
        const BYTE hyperionSig1[] = {0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC0};
        const BYTE hyperionSig2[] = {0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xD9};
        const BYTE hyperionSig3[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20};
        
        for (SIZE_T i = 0; i < size - sizeof(hyperionSig1); i++) {
            if (!IsBadReadPtr(base + i, sizeof(hyperionSig1))) {
                if (memcmp(base + i, hyperionSig1, sizeof(hyperionSig1)) == 0) {
                    return true;
                }
            }
            if (!IsBadReadPtr(base + i, sizeof(hyperionSig2))) {
                if (memcmp(base + i, hyperionSig2, sizeof(hyperionSig2)) == 0) {
                    return true;
                }
            }
            if (!IsBadReadPtr(base + i, sizeof(hyperionSig3))) {
                if (memcmp(base + i, hyperionSig3, sizeof(hyperionSig3)) == 0) {
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

void HyperionEvasion::BypassHyperionChecks() {
    __try {
        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (!ntdll) return;
        
        FARPROC ntQueryInfo = GetProcAddress(ntdll, "NtQueryInformationProcess");
        if (ntQueryInfo && !IsBadWritePtr(ntQueryInfo, 12)) {
            DWORD oldProtect;
            if (VirtualProtect(ntQueryInfo, 12, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                BYTE patch[] = {0x48, 0x31, 0xC0, 0x48, 0xFF, 0xC0, 0xC3};
                memcpy(ntQueryInfo, patch, sizeof(patch));
                VirtualProtect(ntQueryInfo, 12, oldProtect, &oldProtect);
            }
        }
        
        FARPROC ntSetInfo = GetProcAddress(ntdll, "NtSetInformationThread");
        if (ntSetInfo && !IsBadWritePtr(ntSetInfo, 8)) {
            DWORD oldProtect;
            if (VirtualProtect(ntSetInfo, 8, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                BYTE patch[] = {0x48, 0x31, 0xC0, 0xC3};
                memcpy(ntSetInfo, patch, sizeof(patch));
                VirtualProtect(ntSetInfo, 8, oldProtect, &oldProtect);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

void HyperionEvasion::SpoofRobloxMetrics() {
    __try {
        HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
        if (!kernel32) return;
        
        FARPROC getTickCount = GetProcAddress(kernel32, "GetTickCount");
        if (getTickCount && !IsBadWritePtr(getTickCount, 8)) {
            DWORD oldProtect;
            if (VirtualProtect(getTickCount, 8, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                BYTE spoofPatch[] = {0xB8, 0x00, 0x10, 0x00, 0x00, 0xC3};
                memcpy(getTickCount, spoofPatch, sizeof(spoofPatch));
                VirtualProtect(getTickCount, 8, oldProtect, &oldProtect);
            }
        }
        
        FARPROC queryPerf = GetProcAddress(kernel32, "QueryPerformanceCounter");
        if (queryPerf && !IsBadWritePtr(queryPerf, 8)) {
            DWORD oldProtect;
            if (VirtualProtect(queryPerf, 8, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                BYTE spoofPatch[] = {0x48, 0x31, 0xC0, 0x48, 0xFF, 0xC0, 0xC3};
                memcpy(queryPerf, spoofPatch, sizeof(spoofPatch));
                VirtualProtect(queryPerf, 8, oldProtect, &oldProtect);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool BehaviorMimicry::Initialize() {
    return true;
}

void BehaviorMimicry::MimicLegitimateUser() {
    __try {
        SimulateMouseMovement();
        RandomizeActionTimings();
        SimulateKeystrokes();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

void BehaviorMimicry::SimulateMouseMovement() {
    __try {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(-5, 5);
        
        POINT cursor;
        if (GetCursorPos(&cursor)) {
            int deltaX = dis(gen);
            int deltaY = dis(gen);
            
            SetCursorPos(cursor.x + deltaX, cursor.y + deltaY);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

void BehaviorMimicry::SimulateKeystrokes() {
    __try {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> keyDis(0x41, 0x5A);
        
        if (gen() % 1000 == 0) {
            BYTE vk = (BYTE)keyDis(gen);
            keybd_event(vk, 0, 0, 0);
            Sleep(50);
            keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

void BehaviorMimicry::RandomizeActionTimings() {
    __try {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> timeDis(10, 100);
        
        Sleep(timeDis(gen));
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool SignatureEvasion::Initialize() {
    return true;
}

void SignatureEvasion::MutateSignatures() {
    __try {
        if (m_originalSignatures.empty()) return;
        
        m_mutatedSignatures = m_originalSignatures;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> byteDis(0, 255);
        
        for (size_t i = 0; i < m_mutatedSignatures.size(); i += 4) {
            if (i + 3 < m_mutatedSignatures.size()) {
                BYTE randomByte = (BYTE)byteDis(gen);
                m_mutatedSignatures[i] ^= randomByte;
                m_mutatedSignatures[i + 1] = (m_mutatedSignatures[i + 1] << 1) | (m_mutatedSignatures[i + 1] >> 7);
                m_mutatedSignatures[i + 2] += randomByte;
                m_mutatedSignatures[i + 3] ^= 0xAA;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

void SignatureEvasion::PolymorphicTransformation() {
    __try {
        HMODULE currentModule = GetModuleHandleA(NULL);
        if (!currentModule) return;
        
        MODULEINFO modInfo = {};
        if (!GetModuleInformation(GetCurrentProcess(), currentModule, &modInfo, sizeof(modInfo))) {
            return;
        }
        
        BYTE* base = (BYTE*)modInfo.lpBaseOfDll;
        SIZE_T size = modInfo.SizeOfImage;
        
        if (!base || IsBadReadPtr(base, size)) return;
        
        for (SIZE_T i = 0; i < size - 16; i += 16) {
            if (!IsBadWritePtr(base + i, 16)) {
                DWORD oldProtect;
                if (VirtualProtect(base + i, 16, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    if (base[i] == 0x90 && base[i + 1] == 0x90) {
                        base[i] = 0x40;
                        base[i + 1] = 0x90;
                    }
                    
                    if (base[i] == 0xCC) {
                        base[i] = 0x90;
                    }
                    
                    VirtualProtect(base + i, 16, oldProtect, &oldProtect);
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool SignatureEvasion::AvoidKnownPatterns() {
    __try {
        const BYTE knownPattern1[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC};
        const BYTE knownPattern2[] = {0x48, 0x89, 0x5C, 0x24, 0x08};
        const BYTE knownPattern3[] = {0x40, 0x53, 0x48, 0x83, 0xEC, 0x20};
        
        HMODULE currentModule = GetModuleHandleA(NULL);
        if (!currentModule) return false;
        
        MODULEINFO modInfo = {};
        if (!GetModuleInformation(GetCurrentProcess(), currentModule, &modInfo, sizeof(modInfo))) {
            return false;
        }
        
        BYTE* base = (BYTE*)modInfo.lpBaseOfDll;
        SIZE_T size = modInfo.SizeOfImage;
        
        if (!base || IsBadReadPtr(base, size)) return false;
        
        for (SIZE_T i = 0; i < size - sizeof(knownPattern1); i++) {
            if (!IsBadReadPtr(base + i, sizeof(knownPattern1))) {
                if (memcmp(base + i, knownPattern1, sizeof(knownPattern1)) == 0) {
                    DWORD oldProtect;
                    if (VirtualProtect(base + i, sizeof(knownPattern1), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                        base[i] = 0x90;
                        base[i + 1] = 0x90;
                        VirtualProtect(base + i, sizeof(knownPattern1), oldProtect, &oldProtect);
                    }
                }
            }
            
            if (!IsBadReadPtr(base + i, sizeof(knownPattern2))) {
                if (memcmp(base + i, knownPattern2, sizeof(knownPattern2)) == 0) {
                    DWORD oldProtect;
                    if (VirtualProtect(base + i, sizeof(knownPattern2), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                        base[i] = 0x90;
                        base[i + 1] = 0x90;
                        VirtualProtect(base + i, sizeof(knownPattern2), oldProtect, &oldProtect);
                    }
                }
            }
            
            if (!IsBadReadPtr(base + i, sizeof(knownPattern3))) {
                if (memcmp(base + i, knownPattern3, sizeof(knownPattern3)) == 0) {
                    DWORD oldProtect;
                    if (VirtualProtect(base + i, sizeof(knownPattern3), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                        base[i] = 0x90;
                        base[i + 1] = 0x90;
                        VirtualProtect(base + i, sizeof(knownPattern3), oldProtect, &oldProtect);
                    }
                }
            }
        }
        
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

}
}
