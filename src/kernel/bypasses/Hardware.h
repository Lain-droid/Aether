#pragma once

#include <ntifs.h>

namespace AetherVisor {
    namespace Bypasses {

        /*
         * This module provides the infrastructure for hardware-level manipulations,
         * including spoofing identifiers and using hardware features for stealth.
         */
        namespace Hardware {

            // Spoofs various hardware identifiers (Disk, SMBIOS, GPU, etc.)
            // by filtering and modifying the data returned by I/O requests.
            NTSTATUS SpoofIdentifiers();

            // Leverages Direct Memory Access (DMA) to read/write target process memory
            // from a separate, controlled device (e.g., a PCIe card).
            // This is a theoretical placeholder for a physical DMA device.
            NTSTATUS ExecuteDmaAttack(PVOID pTargetAddress, PVOID pBuffer, SIZE_T size, BOOLEAN isWrite);

            // Manipulates hardware breakpoints to evade detection.
            NTSTATUS SetHardwareBreakpoint(PVOID pAddress, UCHAR trigger);

            // Clears a previously set hardware breakpoint.
            NTSTATUS ClearHardwareBreakpoint(PVOID pAddress);

            // Leverages GPU for computation to hide activity from CPU-based scanners.
            NTSTATUS OffloadComputationToGpu(PVOID pComputationFunction);

            // Initializes the hardware bypass module.
            NTSTATUS Initialize();
        }
    }
}
