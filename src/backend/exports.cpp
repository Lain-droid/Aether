#include <windows.h>
#include "InjectionEngine.h"

extern "C" {
    __declspec(dllexport) bool InjectIntoRoblox() {
        using namespace Aether;
        
        if (InjectionEngine::Initialize() != InjectionResult::SUCCESS) {
            return false;
        }
        
        InjectionResult result = InjectionEngine::InjectIntoTarget(L"RobloxPlayerBeta.exe");
        
        if (result != InjectionResult::SUCCESS) {
            InjectionEngine::Cleanup();
            return false;
        }
        
        return true;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_PROCESS_DETACH:
            Aether::InjectionEngine::Cleanup();
            break;
    }
    return TRUE;
}
