#pragma once

#ifdef _WIN32
#include <Windows.h>
using ByteType = BYTE;
#else
using HMODULE = void*;
using ByteType = unsigned char;
#endif
#include <vector>

namespace AetherVisor {
    namespace Security {

        /**
         * @class SignatureScanner
         * @brief A utility for finding patterns of bytes in memory.
         *
         * This class is essential for making the tool resilient to game updates.
         * Instead of relying on fixed function addresses or exports, we can find
         * functions and data by searching for their unique byte patterns (signatures).
         */
        class SignatureScanner {
        public:
            /**
             * @brief Scans a memory region for a given byte pattern.
             * @param moduleBase The base address of the module to scan.
             * @param pattern A string representing the byte pattern. Use '?' as a wildcard.
             *        Example: "55 8B EC 83 E4 F8 83 EC ?? 56 57"
             * @return The address where the pattern was found, or nullptr if not found.
             */
            static void* FindPattern(HMODULE moduleBase, const char* pattern);

        private:
            SignatureScanner() = default;

            // Helper to convert the string pattern to bytes and a mask.
            static bool ParsePattern(const char* pattern, std::vector<ByteType>& outBytes, std::vector<bool>& outMask);
        };

    } // namespace Security
} // namespace AetherVisor
