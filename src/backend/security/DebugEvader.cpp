#include "DebugEvader.h"
#include <Windows.h>
#include <chrono>
#include <winternl.h> // For PEB structures
#include <vector>
#include <string>

namespace AetherVisor {
    namespace Security {

        bool DebugEvader::IsDebugging() {
            if (IsDebuggerPresentCheck()) return true;
            if (CheckRemoteDebuggerPresentCheck()) return true;
            if (TimingCheck()) return true;
            if (CheckHardwareBreakpoints()) return true;
            if (CheckNtGlobalFlag()) return true;
            if (CheckForLoadedModules()) return true;

            return false;
        }

        bool DebugEvader::IsDebuggerPresentCheck() { return IsDebuggerPresent(); }

        bool DebugEvader::CheckRemoteDebuggerPresentCheck() {
            BOOL isDebugging = FALSE;
            CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebugging);
            return isDebugging;
        }

        bool DebugEvader::TimingCheck() {
            auto startTime = std::chrono::high_resolution_clock::now();
            volatile int temp = 0;
            for (int i = 0; i < 10000; ++i) temp += i;
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            if (duration > 50) return true;
            return false;
        }

        bool DebugEvader::CheckHardwareBreakpoints() {
            CONTEXT ctx = {};
            ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
            if (GetThreadContext(GetCurrentThread(), &ctx)) {
                if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0) return true;
            }
            return false;
        }

        bool DebugEvader::CheckNtGlobalFlag() {
            #ifdef _WIN64
            PEB* pPeb = (PEB*)__readgsqword(0x60);
            #else
            PEB* pPeb = (PEB*)__readfsdword(0x30);
            #endif
            if (pPeb->NtGlobalFlag & 0x70) return true;
            return false;
        }

        bool DebugEvader::CheckForLoadedModules() {
            const std::vector<std::string> badModules = { "x64dbg.dll", "Scylla.dll", "TitanHide.dll" };
            for (const auto& moduleName : badModules) {
                if (GetModuleHandleA(moduleName.c_str()) != NULL) {
                    return true;
                }
            }
            return false;
        }

    } // namespace Security
} // namespace AetherVisor
