#include "SignatureScanner.h"
#include <sstream>
#include <Psapi.h> // For GetModuleInformation

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

        void* SignatureScanner::FindPattern(HMODULE moduleBase, const char* pattern) {
            if (!moduleBase || !pattern) {
                return nullptr;
            }

            MODULEINFO moduleInfo;
            if (!GetModuleInformation(GetCurrentProcess(), moduleBase, &moduleInfo, sizeof(moduleInfo))) {
                return nullptr;
            }

            std::vector<BYTE> patternBytes;
            std::vector<bool> patternMask;
            if (!ParsePattern(pattern, patternBytes, patternMask)) {
                return nullptr;
            }

            const BYTE* scanStart = reinterpret_cast<const BYTE*>(moduleBase);
            const size_t scanSize = moduleInfo.SizeOfImage;
            const size_t patternSize = patternBytes.size();

            for (size_t i = 0; i < scanSize - patternSize; ++i) {
                bool found = true;
                for (size_t j = 0; j < patternSize; ++j) {
                    // If the mask at this position is false, we do a byte comparison.
                    // If it's true (wildcard), we skip the check.
                    if (!patternMask[j] && scanStart[i + j] != patternBytes[j]) {
                        found = false;
                        break;
                    }
                }
                if (found) {
                    return const_cast<BYTE*>(&scanStart[i]);
                }
            }

            return nullptr;
        }

    } // namespace Security
} // namespace AetherVisor
