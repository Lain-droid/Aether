#pragma once

#include <ntifs.h>

namespace AetherVisor {
    namespace Bypasses {

        /*
         * This module provides the infrastructure for concealing the AetherVisor kernel driver
         * from various system enumeration and detection mechanisms.
         */
        namespace Driver {

            // Unlinks the driver from the PsLoadedModuleList to hide from standard enumeration.
            NTSTATUS HideFromModuleList(PDRIVER_OBJECT pDriverObject);

            // Erases the driver's entry from the kernel's handle table.
            NTSTATUS EraseHandleTableEntry(PDRIVER_OBJECT pDriverObject);

            // Removes the driver's name and device information from the object manager.
            NTSTATUS ObfuscateObjectName(PDRIVER_OBJECT pDriverObject);

            // Modifies the driver's memory permissions to prevent scanning after initialization.
            NTSTATUS HardenMemoryPermissions(PDRIVER_OBJECT pDriverObject);

            // Erases the PE header of the driver from memory to thwart memory scanners.
            NTSTATUS ErasePeHeaders(PDRIVER_OBJECT pDriverObject);

            // Detects and detaches from common kernel debuggers.
            NTSTATUS DetachFromDebugger();

            // Initializes all bypasses for the driver.
            NTSTATUS Initialize(PDRIVER_OBJECT pDriverObject);
        }
    }
}
