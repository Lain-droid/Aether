#pragma once

#include <string>
#include <functional>

namespace AetherVisor {
    namespace Payload {

        class Hooks {
        public:
            // Installs hooks on Roblox's console functions
            static bool Install();

            // Removes all installed hooks
            static void Uninstall();
        };

    } // namespace Payload
} // namespace AetherVisor
