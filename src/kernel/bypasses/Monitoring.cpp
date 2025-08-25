#include "Monitoring.h"

// NOTE: These are placeholder implementations.

namespace AetherVisor {
    namespace Bypasses {
        namespace Monitoring {

            NTSTATUS DisableEtwProvider(const GUID& providerGuid) {
                UNREFERENCED_PARAMETER(providerGuid);
                DbgPrintEx(0, 0, "[AetherVisor] Bypass: DisableEtwProvider called.\n");
                return STATUS_NOT_IMPLEMENTED;
            }

            NTSTATUS SuppressDebugPrints() {
                DbgPrintEx(0, 0, "[AetherVisor] Bypass: SuppressDebugPrints called.\n");
                return STATUS_NOT_IMPLEMENTED;
            }

            NTSTATUS RemoveProcessCreationCallbacks() {
                DbgPrintEx(0, 0, "[AetherVisor] Bypass: RemoveProcessCreationCallbacks called.\n");
                return STATUS_NOT_IMPLEMENTED;
            }

            NTSTATUS RemoveThreadCreationCallbacks() {
                DbgPrintEx(0, 0, "[AetherVisor] Bypass: RemoveThreadCreationCallbacks called.\n");
                return STATUS_NOT_IMPLEMENTED;
            }

            NTSTATUS RemoveImageLoadCallbacks() {
                DbgPrintEx(0, 0, "[AetherVisor] Bypass: RemoveImageLoadCallbacks called.\n");
                return STATUS_NOT_IMPLEMENTED;
            }

            NTSTATUS Initialize() {
                DbgPrintEx(0, 0, "[AetherVisor] Monitoring Bypass module initialized.\n");
                return STATUS_SUCCESS;
            }
        }
    }
}
