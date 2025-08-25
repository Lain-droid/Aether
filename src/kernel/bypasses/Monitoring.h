#pragma once

#include <ntifs.h>

namespace AetherVisor {
    namespace Bypasses {

        /*
         * This module provides the infrastructure for disabling or deceiving system-wide
         * monitoring and logging facilities used by anti-cheats.
         */
        namespace Monitoring {

            // Finds and disables specific Event Tracing for Windows (ETW) providers
            // that are known to be used for monitoring process and thread behavior.
            NTSTATUS DisableEtwProvider(const GUID& providerGuid);

            // Patches the function responsible for DbgPrint/DbgPrintEx to prevent
            // our driver's debug messages from being logged.
            NTSTATUS SuppressDebugPrints();

            // Detects and removes kernel-level callbacks set by anti-cheat drivers,
            // such as process creation notifications (PsSetCreateProcessNotifyRoutine).
            NTSTATUS RemoveProcessCreationCallbacks();

            // Detects and removes thread creation notifications.
            NTSTATUS RemoveThreadCreationCallbacks();

            // Detects and removes image load notifications.
            NTSTATUS RemoveImageLoadCallbacks();

            // Initializes all monitoring bypasses.
            NTSTATUS Initialize();
        }
    }
}
