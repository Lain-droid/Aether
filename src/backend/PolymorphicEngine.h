#pragma once

#include <vector>

namespace AetherVisor {
    namespace Backend {

        /**
         * @class PolymorphicEngine
         * @brief A simple engine for mutating payloads to evade signature-based detection.
         *
         * This class implements basic polymorphism. Before injection, the payload
         * can be passed through this engine to alter its binary structure slightly,
         * changing its hash and making it harder for static analysis tools to recognize.
         */
        class PolymorphicEngine {
        public:
            // Gets the singleton instance of the PolymorphicEngine.
            static PolymorphicEngine& GetInstance();

            /**
             * @brief Applies a mutation to the given payload.
             * @param payload A vector of bytes representing the payload DLL.
             */
            void Mutate(std::vector<unsigned char>& payload);

        private:
            PolymorphicEngine() = default;
            ~PolymorphicEngine() = default;
            PolymorphicEngine(const PolymorphicEngine&) = delete;
            PolymorphicEngine& operator=(const PolymorphicEngine&) = delete;

            // Adds a random number of NOP instructions to the end of the payload.
            void AppendNopSled(std::vector<unsigned char>& payload);

            // Adds a sequence of random, valid-but-useless instructions to the payload.
            void AddJunkInstructions(std::vector<unsigned char>& payload);
        };

    } // namespace Backend
} // namespace AetherVisor
