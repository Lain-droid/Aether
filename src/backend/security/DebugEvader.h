#pragma once

namespace AetherVisor {
    namespace Security {

        // A collection of static methods to detect debuggers.
        class DebugEvader {
        public:
            // Performs all available anti-debugging checks.
            static bool IsDebugging();

        private:
            // Checks using the standard WinAPI function.
            static bool IsDebuggerPresentCheck();

            // Checks for a remote debugger attached to the process.
            static bool CheckRemoteDebuggerPresentCheck();

            // A simple timing check to detect debugger overhead.
            static bool TimingCheck();

            // Checks for hardware breakpoints in debug registers.
            static bool CheckHardwareBreakpoints();

            // Checks the PEB's NtGlobalFlag.
            static bool CheckNtGlobalFlag();
        };

    } // namespace Security
} // namespace AetherVisor
