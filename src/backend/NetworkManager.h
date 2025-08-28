#pragma once

#include <chrono>
#include <vector>

namespace AetherVisor {
    namespace Payload {

        enum class NetworkMode {
            PASS_THROUGH, // Default mode, does nothing but hook.
            PROFILING,    // Gathers statistics about network traffic.
            MIMICKING     // Alters traffic to match the gathered profile.
        };

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

            /**
             * @brief Sets the operational mode of the NetworkManager.
             * @param newMode The mode to switch to (Profiling or Mimicking).
             */
            static void SetMode(NetworkMode newMode);

        private:
            // --- Traffic Mimicry Internals ---
            struct TrafficProfile {
                double avgPacketSize = 1024.0;
                double avgPacketIntervalMs = 50.0;
                long packetCount = 0;
            };

            static NetworkMode m_mode;
            static TrafficProfile m_profile;
            static std::chrono::steady_clock::time_point m_lastSendTime;
            static std::vector<char> m_sendBuffer;
        };

    } // namespace Payload
} // namespace AetherVisor
