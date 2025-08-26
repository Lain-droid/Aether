#include "DebugEvader.h"
#include <Windows.h>
#include <chrono>

namespace AetherVisor {
    namespace Security {

        bool DebugEvader::IsDebugging() {
            if (IsDebuggerPresentCheck()) return true;
            if (CheckRemoteDebuggerPresentCheck()) return true;
            if (TimingCheck()) return true;

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

            // Perform some trivial work that should be fast.
            // A debugger stepping through this will cause a significant delay.
            volatile int temp = 0;
            for (int i = 0; i < 10000; ++i) {
                temp += i;
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

            // If it took longer than, say, 50ms, something is suspicious.
            if (duration > 50) {
                return true;
            }
            return false;
        }

    } // namespace Security
} // namespace AetherVisor
