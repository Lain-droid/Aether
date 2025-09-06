#pragma once
#include <Windows.h>
#include <string>
#include <vector>

namespace AetherVisor {
namespace Security {

class ProcessStealth {
public:
    static bool Initialize();
    
    static bool HideFromProcessList();
    static bool SpoofProcessName(const std::string& fakeName);
    static bool MaskProcessMemory();
    static bool HideThreads();
    
    static bool SpoofPEB();
    static bool ModifyProcessParameters();

private:
    static bool m_active;
};

class NetworkStealth {
public:
    static bool Initialize();
    
    static void ObfuscateTraffic(std::vector<BYTE>& data);
    static void DeobfuscateTraffic(std::vector<BYTE>& data);
    
    static bool EnableSSLPinning();
    static bool DetectNetworkMonitoring();
    static void GenerateRealisticTraffic();

private:
    static DWORD m_obfuscationKey;
};

class CodeObfuscation {
    friend class SecurityManager;
public:
    static bool Initialize();
    
    static void MutateCodeAtRuntime(void* codePtr, size_t size);
    static void EncryptCriticalFunctions();
    static void ImplementControlFlowFlattening();
    
    static void* CreatePolymorphicCode(void* originalCode, size_t size);
    static void DestroyPolymorphicCode(void* polymorphicCode);

private:
    static std::vector<void*> m_mutatedRegions;
};

class EnvironmentDetection {
public:
    static bool Initialize();
    
    static bool DetectVirtualMachine();
    static bool DetectSandboxEnvironment();
    static bool DetectEmulation();
    static bool CheckHardwareFingerprints();
    
    static bool DetectAnalysisTools();
    static bool IsBeingAnalyzed();

private:
    static bool CheckCPUID();
    static bool CheckRegistryKeys();
    static bool CheckRunningProcesses();
};

}
}
