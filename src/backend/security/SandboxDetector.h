#pragma once

namespace AetherVisor {
    namespace Security {

        // A collection of static methods to detect sandbox and VM environments.
        class SandboxDetector {
        public:
            // Performs all available sandbox/VM checks.
            static bool IsInSandbox();

        private:
            // Checks for the presence of Sandboxie DLL.
            static bool CheckForSandboxie();

            // Checks for tell-tale signs of VMware.
            static bool CheckForVMware();

            // Checks for tell-tale signs of VirtualBox.
            static bool CheckForVirtualBox();
        };

    } // namespace Security
} // namespace AetherVisor
