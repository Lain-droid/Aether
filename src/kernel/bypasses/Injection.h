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

            // Creates a legitimate process in a suspended state and replaces its memory
            // with our payload before resuming it.
            NTSTATUS ProcessHollowing(PWCHAR pTargetPath, PVOID pPayload, SIZE_T payloadSize);

            // Finds and utilizes a "code cave" (a block of unused memory) in the target
            // process to write and execute the payload.
            NTSTATUS CodeCaveInjection(PEPROCESS pProcess, PVOID pPayload, SIZE_T payloadSize);

            // Builds a chain of existing code gadgets in the target process to execute
            // complex operations without injecting new code (Return-Oriented Programming).
            NTSTATUS ExecuteRopChain(PEPROCESS pProcess, PVOID pRopChain, SIZE_T chainSize);

            // Executes a system call directly from kernel mode on behalf of a user process
            // to bypass user-mode API hooks.
            NTSTATUS ExecuteDirectSyscall(PEPROCESS pProcess, ULONG syscallIndex, PVOID pArguments);

            // Main function to select and perform an injection.
            // The 'method' parameter could be an enum to select one of the above.
            NTSTATUS InjectPayload(PEPROCESS pProcess, PVOID pPayload, SIZE_T payloadSize, ULONG injectionMethod);

        }
    }
}
