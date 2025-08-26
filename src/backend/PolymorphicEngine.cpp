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

            // Chain multiple mutation techniques for a more complex result.
            SubstituteInstructions(payload);
            AppendNopSled(payload);
            AddJunkInstructions(payload);
        }

        void PolymorphicEngine::SubstituteInstructions(std::vector<unsigned char>& payload) {
            // This is a naive implementation for demonstration. A real one would need
            // a full disassembler engine to safely replace instructions without
            // corrupting relative offsets.
            for (size_t i = 0; i < payload.size(); ++i) {
                // Look for 'inc eax' (opcode 0x40)
                if (payload[i] == 0x40) {
                    // Replace with 'add eax, 1' (0x83 0xC0 0x01). This changes the length.
                    // To avoid corruption in this simplified demo, we will replace it
                    // with a 3-byte NOP (0x0F 0x1F 0x00) which has the same effect
                    // of changing the code signature and length without breaking things.
                    payload[i] = 0x0F;
                    payload.insert(payload.begin() + i + 1, {0x1F, 0x00});
                    // Advance past the newly inserted bytes
                    i += 2;
                }
            }
        }

        void PolymorphicEngine::AddJunkInstructions(std::vector<unsigned char>& payload) {
            // A list of valid but mostly useless x86 instructions (opcodes).
            const std::vector<std::vector<unsigned char>> junkInstructions = {
                {0x50, 0x58},       // push eax; pop eax
                {0x51, 0x59},       // push ecx; pop ecx
                {0x87, 0xC9},       // xchg ecx, ecx
                {0x87, 0xD2},       // xchg edx, edx
                {0x48},             // dec eax
                {0x40}              // inc eax
            };

            auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::mt19937 gen(static_cast<unsigned int>(seed));
            std::uniform_int_distribution<> count_distrib(3, 8); // Add 3 to 8 junk instructions
            std::uniform_int_distribution<> instr_distrib(0, junkInstructions.size() - 1);

            int instructionCount = count_distrib(gen);
            for (int i = 0; i < instructionCount; ++i) {
                const auto& instr = junkInstructions[instr_distrib(gen)];
                payload.insert(payload.end(), instr.begin(), instr.end());
            }
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
