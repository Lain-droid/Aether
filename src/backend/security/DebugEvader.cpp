#include "DebugEvader.h"
#include <Windows.h>
#include <chrono>
#include <winternl.h> // For PEB structures

namespace AetherVisor {
    namespace Security {

        bool DebugEvader::IsDebugging() {
            if (IsDebuggerPresentCheck()) return true;
            if (CheckRemoteDebuggerPresentCheck()) return true;
            if (TimingCheck()) return true;
            if (CheckHardwareBreakpoints()) return true;
            if (CheckNtGlobalFlag()) return true;

            return false;
        }

        bool DebugEvader::IsDebuggerPresentCheck() {
            return IsDebuggerPresent();
        }

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

            HANDLE hThread = GetCurrentThread();
            if (GetThreadContext(hThread, &ctx)) {
                if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0) {
                    return true;
                }
            }
            return false;
        }

        bool DebugEvader::CheckNtGlobalFlag() {
            // This method is architecture-specific.
            #ifdef _WIN64
            PEB* pPeb = (PEB*)__readgsqword(0x60);
            #else
            PEB* pPeb = (PEB*)__readfsdword(0x30);
            #endif

            // Check for debugger flags
            if (pPeb->NtGlobalFlag & 0x70) {
                return true;
            }
            return false;
        }

    } // namespace Security
} // namespace AetherVisor
