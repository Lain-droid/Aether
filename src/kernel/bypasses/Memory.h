#pragma once

#include <ntifs.h>

namespace AetherVisor {
    namespace Bypasses {

        /*
         * This module provides the infrastructure for manipulating and concealing memory
         * within the target process to evade anti-cheat memory scans.
         */
        namespace Memory {

            // Unlinks the injected module from the PEB's InLoadOrderModuleList,
            // InInitializationOrderModuleList, and InProgressModuleList.
            NTSTATUS UnlinkModuleFromPeb(PEPROCESS pProcess, PVOID pModuleBase);

            // Spoofs the memory page permissions in the VAD (Virtual Address Descriptor) tree
            // to make executable pages appear as non-executable.
            NTSTATUS SpoofPageProtections(PEPROCESS pProcess, PVOID pBaseAddress, SIZE_T size);

            // Erases the PE headers of the manually mapped payload from memory.
            NTSTATUS ErasePayloadPeHeaders(PEPROCESS pProcess, PVOID pModuleBase);

            // Hides memory pages from being read by external processes.
            NTSTATUS SetPageGuard(PEPROCESS pProcess, PVOID pBaseAddress, SIZE_T size);

            // Provides a secure way to read memory from the target process, bypassing hooks.
            NTSTATUS SafeReadProcessMemory(PEPROCESS pProcess, PVOID pAddress, PVOID pBuffer, SIZE_T size);

            // Provides a secure way to write memory to the target process, bypassing hooks.
            NTSTATUS SafeWriteProcessMemory(PEPROCESS pProcess, PVOID pAddress, PVOID pBuffer, SIZE_T size);

        }
    }
}
