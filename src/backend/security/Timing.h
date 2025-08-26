#pragma once

#include <thread>
#include <chrono>
#include <random>

namespace AetherVisor {
    namespace Security {

        class Timing {
        public:
            // Sleeps for a short, random duration to make execution timing less predictable.
            static void RandomDelay(int min_ms = 10, int max_ms = 50) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distrib(min_ms, max_ms);

                std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));
            }
        };

    } // namespace Security
} // namespace AetherVisor
