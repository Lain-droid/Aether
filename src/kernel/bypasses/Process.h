#pragma once

#include <ntifs.h>

namespace AetherVisor {
    namespace Bypasses {

        /*
         * This module provides the infrastructure for process-related manipulations,
         * such as protecting the executor's own process or altering process attributes.
         */
        namespace Process {

            // Elevates the token of a given process to SYSTEM level.
            NTSTATUS ElevateProcessToken(PEPROCESS pProcess);

            // Strips a process of a specific handle, for example a handle
            // that an anti-cheat has to our executor process.
            NTSTATUS StripProcessHandle(PEPROCESS pTargetProcess, HANDLE hTargetHandle);

            // Sets the ProtectedProcess flag on a given process, making it very
            // difficult for other user-mode processes to inspect or terminate it.
            NTSTATUS ProtectProcess(PEPROCESS pProcess);

            // Hides a process from standard enumeration by unlinking it from the
            // active process list. Note: This is a highly aggressive technique.
            NTSTATUS HideProcess(PEPROCESS pProcess);

            // Changes the parent process ID (PPID) of a process to a more
            // innocuous one, like explorer.exe.
            NTSTATUS SpoofParentProcess(PEPROCESS pChildProcess, PEPROCESS pNewParentProcess);
        }
    }
}
