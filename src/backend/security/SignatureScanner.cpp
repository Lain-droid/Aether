#include "SignatureScanner.h"
#include "SecurityTypes.h"
#include <sstream>
#ifdef _WIN32
#include <Psapi.h> // For GetModuleInformation
#endif

namespace AetherVisor::Security {

    bool SignatureScanner::ParsePattern(const char* pattern, std::vector<ByteType>& outBytes, std::vector<bool>& outMask) {
        std::stringstream ss(pattern);
        std::string byteStr;
        while (ss >> byteStr) {
            if (byteStr == "?" || byteStr == "??") {
                outBytes.push_back(0);
                outMask.push_back(true);
            } else {
                outBytes.push_back(static_cast<ByteType>(std::stoi(byteStr, nullptr, 16)));
                outMask.push_back(false);
            }
        }
        return !outBytes.empty();
    }

    void* SignatureScanner::FindPattern(void* moduleBase, const char* pattern) {
        if (!moduleBase || !pattern) {
            return nullptr;
        }

        #ifndef _WIN32
        (void)moduleBase; (void)pattern;
        return nullptr;
        #else
        MODULEINFO moduleInfo;
        if (!GetModuleInformation(GetCurrentProcess(), (HMODULE)moduleBase, &moduleInfo, sizeof(moduleInfo))) {
            return nullptr;
        }

        std::vector<ByteType> patternBytes;
        std::vector<bool> patternMask;
        if (!ParsePattern(pattern, patternBytes, patternMask)) {
            return nullptr;
        }

        const ByteType* scanStart = reinterpret_cast<const ByteType*>(moduleBase);
        const size_t scanSize = moduleInfo.SizeOfImage;
        const size_t patternSize = patternBytes.size();

        for (size_t i = 0; i < scanSize - patternSize; ++i) {
            bool found = true;
            for (size_t j = 0; j < patternSize; ++j) {
                if (!patternMask[j] && scanStart[i + j] != patternBytes[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return const_cast<ByteType*>(&scanStart[i]);
            }
        }

        return nullptr;
        #endif
    }

} // namespace AetherVisor::Security
