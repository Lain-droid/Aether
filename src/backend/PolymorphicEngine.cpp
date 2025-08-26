#include "PolymorphicEngine.h"
#include <random>
#include <chrono>

namespace AetherVisor {
    namespace Backend {

        constexpr unsigned char NOP_OPCODE = 0x90;
        constexpr int MIN_NOPS = 16;
        constexpr int MAX_NOPS = 128;

        PolymorphicEngine& PolymorphicEngine::GetInstance() {
            static PolymorphicEngine instance;
            return instance;
        }

        void PolymorphicEngine::Mutate(std::vector<unsigned char>& payload) {
            if (payload.empty()) {
                return;
            }
            // In the future, more mutation techniques could be chained here.
            // For example:
            //   - EncryptRandomSection(payload);
            //   - ObfuscateStrings(payload);
            AppendNopSled(payload);
        }

        void PolymorphicEngine::AppendNopSled(std::vector<unsigned char>& payload) {
            // Seed the random number generator.
            auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::mt19937 gen(static_cast<unsigned int>(seed));
            std::uniform_int_distribution<> distrib(MIN_NOPS, MAX_NOPS);

            int nopCount = distrib(gen);

            // Append the NOPs to the end of the payload.
            payload.insert(payload.end(), nopCount, NOP_OPCODE);
        }

    } // namespace Backend
} // namespace AetherVisor
