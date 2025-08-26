#include "SignatureScanner.h"
#include <sstream>
#include <Psapi.h> // For GetModuleInformation
#include <algorithm> // for std::max

#define ALPHABET_SIZE 256

namespace AetherVisor {
    namespace Security {

        bool SignatureScanner::ParsePattern(const char* pattern, std::vector<BYTE>& outBytes, std::vector<bool>& outMask) {
            std::stringstream ss(pattern);
            std::string byteStr;
            while (ss >> byteStr) {
                if (byteStr == "?" || byteStr == "??") {
                    outBytes.push_back(0);
                    outMask.push_back(true);
                } else {
                    outBytes.push_back(static_cast<BYTE>(std::stoi(byteStr, nullptr, 16)));
                    outMask.push_back(false);
                }
            }
            return !outBytes.empty();
        }

        void SignatureScanner::BuildBadCharTable(const std::vector<BYTE>& pattern, const std::vector<bool>& mask, std::vector<int>& table) {
            table.assign(ALPHABET_SIZE, -1);
            for (size_t i = 0; i < pattern.size(); ++i) {
                // Only use non-wildcard characters for the bad character table
                if (!mask[i]) {
                    table[pattern[i]] = i;
                }
            }
        }

        void* SignatureScanner::FindPattern(HMODULE moduleBase, const char* pattern) {
            if (!moduleBase || !pattern) return nullptr;

            MODULEINFO moduleInfo;
            if (!GetModuleInformation(GetCurrentProcess(), moduleBase, &moduleInfo, sizeof(moduleInfo))) return nullptr;

            std::vector<BYTE> patternBytes;
            std::vector<bool> patternMask;
            if (!ParsePattern(pattern, patternBytes, patternMask)) return nullptr;

            std::vector<int> badCharTable;
            BuildBadCharTable(patternBytes, patternMask, badCharTable);

            const BYTE* scanStart = reinterpret_cast<const BYTE*>(moduleBase);
            const size_t scanSize = moduleInfo.SizeOfImage;
            const size_t patternSize = patternBytes.size();

            int shift = 0;
            while(shift <= (scanSize - patternSize)) {
                int j = patternSize - 1;

                while(j >= 0 && (patternMask[j] || patternBytes[j] == scanStart[shift + j])) {
                    j--;
                }

                if (j < 0) {
                    // Pattern found
                    return (void*)(scanStart + shift);
                } else {
                    // Mismatch occurred at scanStart[shift + j].
                    // Calculate the shift using the bad character rule.
                    int badCharShift = (badCharTable[scanStart[shift + j]] != -1)
                        ? std::max(1, j - badCharTable[scanStart[shift + j]])
                        : j + 1; // Character not in pattern, shift past it.

                    shift += badCharShift;
                }
            }

            return nullptr;
        }

    } // namespace Security
} // namespace AetherVisor
