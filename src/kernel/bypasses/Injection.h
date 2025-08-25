#pragma once

#include <ntifs.h>

namespace AetherVisor {
    namespace Bypasses {

        /*
         * This module provides the infrastructure for executing code within a target process
         * using various kernel-mode injection techniques to bypass anti-cheat detection.
         */
        namespace Injection {

            // A standard, baseline injection method using Asynchronous Procedure Calls (APCs).
            // Less stealthy than others but reliable.
            NTSTATUS QueueUserApc(PEPROCESS pProcess, PVOID pRemoteCode);

            // A more advanced technique that hijacks an existing thread in the target process
            // to execute our code, avoiding thread creation events.
            NTSTATUS HijackThread(PEPROCESS pProcess, PVOID pRemoteCode);

            // Injects code by mapping it into the target process and then creating a
            // new thread that starts at a system function, later redirecting it.
            NTSTATUS MapAndExecute(PEPROCESS pProcess, PVOID pLocalCode, SIZE_T codeSize);

            // A highly advanced technique that leverages system callbacks (e.g., file system)
            // to gain execution within the process context.
            NTSTATUS LeverageCallback(PEPROCESS pProcess, PVOID pRemoteCode);

            // Main function to select and perform an injection.
            // The 'method' parameter could be an enum to select one of the above.
            NTSTATUS InjectPayload(PEPROCESS pProcess, PVOID pPayload, SIZE_T payloadSize, ULONG injectionMethod);

        }
    }
}
