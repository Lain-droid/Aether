#include "NetworkManager.h"
#include "EventManager.h"
#include "AIController.h" // To report network events
#include <WinSock2.h>
#include <string>

// Link against the Winsock library
#pragma comment(lib, "ws2_32.lib")

// Typedefs for the original function pointers
typedef int (WSAAPI *send_t)(SOCKET s, const char* buf, int len, int flags);
typedef int (WSAAPI *recv_t)(SOCKET s, char* buf, int len, int flags);

// Placeholder for getting function addresses.
// In a real scenario, we'd use GetProcAddress on a handle to ws2_32.dll.
send_t original_send = nullptr;
recv_t original_recv = nullptr;

namespace AetherVisor {
    namespace Payload {

        // Our detour for the 'send' function
        int WSAAPI Detour_Send(SOCKET s, const char* buf, int len, int flags) {
            // Report this event to the AI controller
            Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::NETWORK_PACKET_SENT);

            // Placeholder for future logic (packet shaping, mimicry)
            // For now, we just call the original function.

            if (original_send) {
                return original_send(s, buf, len, flags);
            }
            return SOCKET_ERROR;
        }

        // Our detour for the 'recv' function
        int WSAAPI Detour_Recv(SOCKET s, char* buf, int len, int flags) {
            // Report this event to the AI controller
            Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::NETWORK_PACKET_RECEIVED);

            // Placeholder for future logic

            if (original_recv) {
                return original_recv(s, buf, len, flags);
            }
            return SOCKET_ERROR;
        }

        bool NetworkManager::Install() {
            HMODULE hModule = GetModuleHandleA("ws2_32.dll");
            if (!hModule) {
                // Winsock is not loaded, cannot install hooks.
                return false;
            }

            void* sendAddr = (void*)GetProcAddress(hModule, "send");
            void* recvAddr = (void*)GetProcAddress(hModule, "recv");

            if (!sendAddr || !recvAddr) {
                return false;
            }

            auto& eventManager = EventManager::GetInstance();
            bool sendHooked = eventManager.Install(sendAddr, (void*)Detour_Send);
            bool recvHooked = eventManager.Install(recvAddr, (void*)Detour_Recv);

            if (sendHooked && recvHooked) {
                // Store the original function pointers from the event manager's trampoline
                original_send = eventManager.GetOriginal<send_t>(sendAddr);
                original_recv = eventManager.GetOriginal<recv_t>(recvAddr);
                return (original_send != nullptr && original_recv != nullptr);
            }

            return false;
        }

        void NetworkManager::Uninstall() {
            auto& eventManager = EventManager::GetInstance();

            HMODULE hModule = GetModuleHandleA("ws2_32.dll");
            if (!hModule) return;

            void* sendAddr = (void*)GetProcAddress(hModule, "send");
            void* recvAddr = (void*)GetProcAddress(hModule, "recv");

            if (sendAddr) eventManager.Uninstall(sendAddr);
            if (recvAddr) eventManager.Uninstall(recvAddr);
        }

    } // namespace Payload
} // namespace AetherVisor
