#pragma once

namespace AetherVisor {
    namespace Backend {

        class Security {
        public:
            // Applies anti-debugging measures to the current process
            static void ApplyAntiDebug();

            // Applies anti-tampering measures to protect critical functions
            static void ApplyAntiTamper();

            // Checks for the presence of virtual machines or sandboxes
            static bool IsVirtualEnvironment();

        private:
            // Private helpers for specific techniques
            static void CheckForDebugger();
            static void ObfuscateMemory();
        };

    } // namespace Backend
} // namespace AetherVisor
