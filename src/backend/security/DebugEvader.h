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
        };

    } // namespace Security
} // namespace AetherVisor
