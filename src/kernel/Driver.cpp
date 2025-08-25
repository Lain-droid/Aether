#include <ntifs.h>
#include "bypasses/Cryptography.h"
#include "bypasses/Driver.h"
#include "bypasses/Hardware.h"
#include "bypasses/Injection.h"
#include "bypasses/Memory.h"
#include "bypasses/Monitoring.h"
#include "bypasses/Network.h"
#include "bypasses/Process.h"

// Define IOCTL codes that our user-mode app will use
#define IOCTL_INITIALIZE_BYPASSES CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INJECT_PAYLOAD CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_access)

// Forward declarations
VOID DriverUnload(PDRIVER_OBJECT pDriverObject);
NTSTATUS IoControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);

// Entry point for the driver
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath) {
    UNREFERENCED_PARAMETER(pRegistryPath);

    DbgPrintEx(0, 0, "[AetherVisor] DriverEntry called.\n");

    pDriverObject->DriverUnload = DriverUnload;

    // Set up the dispatch routine for IRP_MJ_DEVICE_CONTROL
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;

    // Initialize all our bypass modules
    // In a real scenario, we would check the return status of each
    AetherVisor::Bypasses::Cryptography::Initialize();
    AetherVisor::Bypasses::Driver::Initialize(pDriverObject);
    AetherVisor::Bypasses::Hardware::Initialize();
    AetherVisor::Bypasses::Monitoring::Initialize();

    DbgPrintEx(0, 0, "[AetherVisor] All bypass modules initialized.\n");

    return STATUS_SUCCESS;
}

// Unload routine
VOID DriverUnload(PDRIVER_OBJECT pDriverObject) {
    UNREFERENCED_PARAMETER(pDriverObject);
    DbgPrintEx(0, 0, "[AetherVisor] DriverUnload called. Cleaning up.\n");
    // Cleanup routines for bypasses would be called here
}

// IOCTL handler
NTSTATUS IoControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
    UNREFERENCED_PARAMETER(pDeviceObject);

    PIO_STACK_LOCATION stack = IoGetNextIrpStackLocation(pIrp);
    NTSTATUS status = STATUS_SUCCESS;

    if (!stack) {
        pIrp->IoStatus.Status = STATUS_INTERNAL_ERROR;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        return STATUS_INTERNAL_ERROR;
    }

    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_INITIALIZE_BYPASSES:
            DbgPrintEx(0, 0, "[AetherVisor] IOCTL_INITIALIZE_BYPASSES received.\n");
            // This could re-trigger initialization or other logic
            break;

        case IOCTL_INJECT_PAYLOAD:
            DbgPrintEx(0, 0, "[AetherVisor] IOCTL_INJECT_PAYLOAD received.\n");
            // Logic to get payload from user-mode and call an injection function
            // e.g., AetherVisor::Bypasses::Injection::InjectPayload(...)
            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    pIrp->IoStatus.Status = status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return status;
}
