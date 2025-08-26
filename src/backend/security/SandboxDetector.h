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

            // Checks for common sandbox usernames.
            static bool CheckUsername();

            // Checks if system uptime is suspiciously low.
            static bool CheckUptime();

            // Checks for common VM MAC addresses.
            static bool CheckMACAddress();
        };

    } // namespace Security
} // namespace AetherVisor
