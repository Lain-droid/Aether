#pragma once

#include <ntifs.h>
#include <wsk.h> // For Winsock Kernel

namespace AetherVisor {
    namespace Bypasses {

        /*
         * This module provides the infrastructure for network-level manipulations,
         * such as intercepting, modifying, and faking network packets.
         */
        namespace Network {

            // Intercepts and allows modification of outgoing network packets for a specific process.
            NTSTATUS InterceptOutgoingPackets(PEPROCESS pProcess, PVOID pCallback);

            // Intercepts and allows modification of incoming network packets.
            NTSTATUS InterceptIncomingPackets(PEPROCESS pProcess, PVOID pCallback);

            // Injects a fake network packet into the system's network stack.
            NTSTATUS InjectFakePacket(PVOID pPacketData, SIZE_T packetSize);

            // Implements a partial custom network stack to bypass user-mode filtering.
            // This is a major undertaking.
            NTSTATUS InitializeCustomStack(PEPROCESS pProcess);

            // Disables or spoofs network-based hardware identifiers.
            NTSTATUS SpoofNetworkAdapters();

            // Initializes the network bypass module.
            NTSTATUS Initialize(PDRIVER_OBJECT pDriverObject);
        }
    }
}
