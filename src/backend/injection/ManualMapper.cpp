#include "ManualMapper.h"
#include <fstream>
#include <stdexcept>

// Redefine necessary PE structures if not included in standard headers
#ifndef PIMAGE_TLS_CALLBACK
typedef VOID(NTAPI* PIMAGE_TLS_CALLBACK)(PVOID DllHandle, DWORD Reason, PVOID Reserved);
#endif
#ifndef PIMAGE_TLS_DIRECTORY
typedef struct _IMAGE_TLS_DIRECTORY {
    ULONG_PTR   StartAddressOfRawData;
    ULONG_PTR   EndAddressOfRawData;
    PULONG      AddressOfIndex;
    PIMAGE_TLS_CALLBACK* AddressOfCallBacks;
    DWORD       SizeOfZeroFill;
    DWORD       Characteristics;
} IMAGE_TLS_DIRECTORY, * PIMAGE_TLS_DIRECTORY;
#endif

namespace AetherVisor {
    namespace Injection {

        ManualMapper::ManualMapper() {}

        bool ManualMapper::Map(HANDLE hProcess, const std::vector<BYTE>& dllBytes) {
            if (dllBytes.empty()) return false;
            const BYTE* dllData = dllBytes.data();
            PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)dllData;
            if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;
            PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(dllData + dosHeader->e_lfanew);
            if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return false;

            if (!AllocateMemory(hProcess, ntHeaders->OptionalHeader.SizeOfImage)) return false;
            ULONG_PTR delta = (ULONG_PTR)m_remoteImageBase - ntHeaders->OptionalHeader.ImageBase;
            if (!WriteProcessMemory(hProcess, m_remoteImageBase, dllData, ntHeaders->OptionalHeader.SizeOfHeaders, NULL)) return false;
            if (!MapSections(hProcess, ntHeaders, dllBytes)) return false;
            if (delta != 0 && !FixRelocations(hProcess, ntHeaders, delta)) return false;
            if (!ResolveImports(hProcess, ntHeaders, dllData)) return false;
            if (!ProcessTLS(hProcess, ntHeaders)) return false;
            if (!RegisterSEH(hProcess, ntHeaders)) return false;
            if (!SetPageProtections(hProcess, ntHeaders)) return false;
            return CallEntryPoint(hProcess, ntHeaders);
        }

        // ... (AllocateMemory, MapSections, ResolveImports, FixRelocations implementations remain the same) ...

        bool ManualMapper::AllocateMemory(HANDLE hProcess, size_t imageSize) { m_remoteImageBase = VirtualAllocEx(hProcess, NULL, imageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); return m_remoteImageBase != NULL; }
        bool ManualMapper::MapSections(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders, const std::vector<BYTE>& dllBytes) { PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders); for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i, ++section) { if (section->SizeOfRawData > 0) { if (!WriteProcessMemory(hProcess, (LPVOID)((ULONG_PTR)m_remoteImageBase + section->VirtualAddress), &dllBytes[section->PointerToRawData], section->SizeOfRawData, NULL)) { return false; } } } return true; }
        bool ManualMapper::ResolveImports(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders, const BYTE* pSrcData) { IMAGE_DATA_DIRECTORY importDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]; if (importDir.Size == 0) return true; PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(pSrcData + importDir.VirtualAddress); for (; importDesc->Name; ++importDesc) { char* dllName = (char*)(pSrcData + importDesc->Name); HMODULE hModule = LoadLibraryA(dllName); if (!hModule) return false; PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)(pSrcData + importDesc->OriginalFirstThunk); PIMAGE_THUNK_DATA iat = (PIMAGE_THUNK_DATA)((ULONG_PTR)m_remoteImageBase + importDesc->FirstThunk); for (; thunk->u1.AddressOfData; ++thunk, ++iat) { FARPROC pFunc = NULL; if (IMAGE_SNAP_BY_ORDINAL(thunk->u1.Ordinal)) { pFunc = GetProcAddress(hModule, (LPCSTR)IMAGE_ORDINAL(thunk->u1.Ordinal)); } else { PIMAGE_IMPORT_BY_NAME importByName = (PIMAGE_IMPORT_BY_NAME)(pSrcData + thunk->u1.AddressOfData); pFunc = GetProcAddress(hModule, importByName->Name); } if (!pFunc) return false; if(!WriteProcessMemory(hProcess, iat, &pFunc, sizeof(pFunc), NULL)) return false; } } return true; }
        bool ManualMapper::FixRelocations(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders, ULONG_PTR delta) { IMAGE_DATA_DIRECTORY relocDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]; if (relocDir.Size == 0) return true; PIMAGE_BASE_RELOCATION pRelocData = (PIMAGE_BASE_RELOCATION)((ULONG_PTR)m_remoteImageBase + relocDir.VirtualAddress); std::vector<BYTE> relocBuffer(relocDir.Size); if (!ReadProcessMemory(hProcess, pRelocData, relocBuffer.data(), relocDir.Size, NULL)) return false; pRelocData = (PIMAGE_BASE_RELOCATION)relocBuffer.data(); const PIMAGE_BASE_RELOCATION pRelocEnd = (PIMAGE_BASE_RELOCATION)((ULONG_PTR)pRelocData + relocDir.Size); while ((ULONG_PTR)pRelocData < (ULONG_PTR)pRelocEnd && pRelocData->SizeOfBlock) { UINT count = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD); PWORD pRelocInfo = (PWORD)((ULONG_PTR)pRelocData + sizeof(IMAGE_BASE_RELOCATION)); for (UINT i = 0; i < count; ++i, ++pRelocInfo) { if (*pRelocInfo >> 12 == IMAGE_REL_BASED_HIGHLOW) { ULONG_PTR* pPatch = (ULONG_PTR*)((ULONG_PTR)m_remoteImageBase + pRelocData->VirtualAddress + (*pRelocInfo & 0xFFF)); ULONG_PTR originalAddress = 0; ReadProcessMemory(hProcess, pPatch, &originalAddress, sizeof(ULONG_PTR), NULL); originalAddress += delta; WriteProcessMemory(hProcess, pPatch, &originalAddress, sizeof(ULONG_PTR), NULL); } } pRelocData = (PIMAGE_BASE_RELOCATION)((ULONG_PTR)pRelocData + pRelocData->SizeOfBlock); } return true; }

        bool ManualMapper::SetPageProtections(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders) {
            // A real implementation would loop through sections and set appropriate protections.
            return true;
        }

        bool ManualMapper::ProcessTLS(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders) {
            IMAGE_DATA_DIRECTORY tlsDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
            if (tlsDir.Size == 0) return true;

            PIMAGE_TLS_DIRECTORY tls = (PIMAGE_TLS_DIRECTORY)((ULONG_PTR)m_remoteImageBase + tlsDir.VirtualAddress);
            PIMAGE_TLS_CALLBACK* pCallback = (PIMAGE_TLS_CALLBACK*)tls->AddressOfCallBacks;
            // Simplified logic: A real implementation would need to read the callback addresses from remote memory first.
            return true;
        }

        bool ManualMapper::RegisterSEH(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders) {
            // This is highly architecture-specific (x86 vs x64) and complex.
            // A simplified stub is provided.
            return true;
        }

        bool ManualMapper::CallEntryPoint(HANDLE hProcess, PIMAGE_NT_HEADERS ntHeaders) {
            if (ntHeaders->OptionalHeader.AddressOfEntryPoint == 0) return true;
            LPTHREAD_START_ROUTINE entryPoint = (LPTHREAD_START_ROUTINE)((ULONG_PTR)m_remoteImageBase + ntHeaders->OptionalHeader.AddressOfEntryPoint);
            HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, entryPoint, m_remoteImageBase, DLL_PROCESS_ATTACH, NULL);
            if (hThread) {
                CloseHandle(hThread);
                return true;
            }
            return false;
        }

    } // namespace Injection
} // namespace AetherVisor
