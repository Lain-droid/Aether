#pragma once

namespace AetherVisor {
    namespace Payload {

        /**
         * @class NetworkManager
         * @brief Manages hooks for network-related APIs.
         *
         * This class is the entry point for the "Network Layer". It is responsible
         * for intercepting network traffic to and from the target process.
         * Initially, it will be used for monitoring, but it is the foundation
         * for future features like traffic shaping, mimicry, and camouflage.
         */
        class NetworkManager {
        public:
            /**
             * @brief Installs hooks on common networking functions (e.g., send, recv).
             * @return True if hooks were installed successfully, false otherwise.
             */
            static bool Install();

            /**
             * @brief Removes all installed network hooks.
             */
            static void Uninstall();
        };

    } // namespace Payload
} // namespace AetherVisor
