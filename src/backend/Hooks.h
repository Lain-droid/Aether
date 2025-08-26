#pragma once

#include <string>
#include <functional>

namespace AetherVisor {
    namespace Payload {

        class Hooks {
        public:
            // Type definition for the callback that forwards console output
            using ConsoleOutputCallback = std::function<void(const std::string& output)>;

            // Installs hooks on Roblox's console functions
            static bool Install(ConsoleOutputCallback callback);

            // Removes all installed hooks
            static void Uninstall();
        };

    } // namespace Payload
} // namespace AetherVisor
