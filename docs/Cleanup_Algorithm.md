# AetherVisor: Automatic Cleanup Algorithm

## 1. Objective

The primary goal of the cleanup algorithm is to ensure that when AetherVisor is shut down or an error occurs, all modifications made to the target process (Roblox) and the host system are reverted completely. This process must be **atomic**, **fail-safe**, and leave no detectable traces, preserving system stability and user security.

The cleanup process is triggered automatically in the following scenarios:
- Normal application shutdown initiated by the user.
- Failure during any stage of the injection process.
- The target Roblox process closing unexpectedly.
- The AetherVisor backend process crashing.

## 2. Cleanup Phases

The cleanup is executed in the reverse order of the injection process to ensure dependencies are handled correctly.

### Phase 1: User-Mode Cleanup (Inside Roblox)

This is the first and most critical phase, executed by a dedicated thread within the injected DLL.

1.  **Remove Function Hooks:**
    *   The hooking mechanism (e.g., MinHook) will be used to un-patch all hooked functions (like `print`, `warn`).
    *   The original function prologues are restored, returning the functions to their original state. This must be done first to prevent crashes if the game calls these functions during the cleanup of other components.

2.  **Terminate Scripting Environment:**
    *   Any running Luau threads or scripts are forcefully terminated.
    *   The custom Luau execution environment is destroyed, and all associated memory is deallocated from within the Roblox process.

3.  **Self-Destruction Sequence:**
    *   The cleanup thread frees all memory allocated by the DLL.
    *   Finally, the thread calls `FreeLibraryAndExitThread`, passing the DLL's own module handle. This erases the main DLL image from memory and terminates the thread simultaneously, effectively making the payload vanish from the target process.

### Phase 2: Backend Cleanup

This phase is executed by the main AetherVisor C++ backend process.

1.  **Close Process Handle:**
    *   The handle to the Roblox process, which was held by the backend, is closed using `CloseHandle`.

2.  **Shutdown IPC Server:**
    *   The named pipe server is shut down, disconnecting any active frontend client.

3.  **Request Driver Cleanup:**
    *   The backend sends a final IOCTL command to the kernel-mode driver, signaling it to perform its own cleanup.

### Phase 3: Kernel-Mode Cleanup (The Driver)

This is the final phase, ensuring the lowest-level system modifications are reverted.

1.  **Re-enable Kernel Protections:**
    *   Any kernel-level bypasses that were applied (e.g., disabling ETW, unpatching anti-cheat driver functions) are carefully reversed. The original state of these components is restored.

2.  **Unload the Driver:**
    *   The driver performs its final cleanup, deallocating any memory it used.
    *   The backend, after receiving confirmation from the driver, issues the command to unload the driver from the system. The driver's entry is removed from the system's loaded module list.

## 3. Error Handling and Fail-Safes

- **Forced Cleanup:** The backend will monitor the Roblox process. If it detects that the process has terminated unexpectedly, it will immediately trigger Phases 2 and 3 of the cleanup, assuming Phase 1 is no longer possible.
- **Blue Screen of Death (BSOD) Prevention:** The kernel driver is the most sensitive component. All its operations, especially during cleanup, must be wrapped in structured exception handlers (`__try`/`__except`). Any failure during kernel cleanup should be logged (if possible) but must not be allowed to crash the system. The priority is system stability over perfect cleanup in a catastrophic failure scenario.
- **Resource Management:** All handles, memory allocations, and other system resources are tracked using RAII (Resource Acquisition Is Initialization) principles in C++ to ensure they are automatically released even if an exception occurs.
