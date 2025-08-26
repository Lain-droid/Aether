#pragma once

#include <Windows.h>
#include <vector>

namespace AetherVisor {
    namespace Injection {

        class ManualMapper {
        public:
            ManualMapper();

            // Maps the specified DLL into the target process.
            bool Map(HANDLE hProcess, const std::vector<BYTE>& dllBytes);

        private:
            // Private helper methods for each stage of the mapping process.
            bool AllocateMemory(HANDLE hProcess, size_t imageSize);
            bool MapSections(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders, const std::vector<BYTE>& dllBytes);
            bool ResolveImports(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders, const BYTE* pSrcData);
            bool FixRelocations(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders, ULONG_PTR delta);
            bool SetPageProtections(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders);
            bool ProcessTLS(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders);
            bool RegisterSEH(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders);
            bool CallEntryPoint(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders);

            // A handle to the allocated memory block in the target process.
            LPVOID m_remoteImageBase = nullptr;
        };

    } // namespace Injection
} // namespace AetherVisor
