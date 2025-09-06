#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif
#include "NetworkManager.h"
#include "EventManager.h"
#include "AIController.h"
#include "security/XorStr.h"
#include "security/SignatureScanner.h"
#include <numeric>

#pragma comment(lib, "ws2_32.lib")

#ifdef _WIN32
typedef int (WSAAPI *send_t)(SOCKET s, const char* buf, int len, int flags);
typedef int (WSAAPI *recv_t)(SOCKET s, char* buf, int len, int flags);
static send_t original_send = nullptr;
static recv_t original_recv = nullptr;
static int WSAAPI Detour_Send(SOCKET s, const char* buf, int len, int flags);
static int WSAAPI Detour_Recv(SOCKET s, char* buf, int len, int flags);
#endif

namespace AetherVisor {
    namespace Payload {

        // Initialize static members
        NetworkMode NetworkManager::m_mode = NetworkMode::PassThrough;
        NetworkManager::TrafficProfile NetworkManager::m_profile;
        std::chrono::steady_clock::time_point NetworkManager::m_lastSendTime = std::chrono::steady_clock::now();
        std::vector<char> NetworkManager::m_sendBuffer;

        void NetworkManager::SetMode(NetworkMode newMode) {
            m_mode = newMode;
            if (newMode == NetworkMode::ProfilingMode) {
                // Reset profile data when starting a new profiling session
                m_profile = {};
            }
        }

        #ifdef _WIN32
        int WSAAPI Detour_Send(SOCKET s, const char* buf, int len, int flags) {
            Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::NETWORK_PACKET_SENT);

            switch (NetworkManager::m_mode) {
                case NetworkMode::ProfilingMode: {
                    auto now = std::chrono::steady_clock::now();
                    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - NetworkManager::m_lastSendTime).count();

                    // Update running averages
                    NetworkManager::m_profile.packetCount++;
                    NetworkManager::m_profile.avgPacketIntervalMs = (NetworkManager::m_profile.avgPacketIntervalMs * (NetworkManager::m_profile.packetCount - 1) + static_cast<double>(elapsedMs)) / NetworkManager::m_profile.packetCount;
                    NetworkManager::m_profile.avgPacketSize = (NetworkManager::m_profile.avgPacketSize * (NetworkManager::m_profile.packetCount - 1) + static_cast<double>(len)) / NetworkManager::m_profile.packetCount;

                    NetworkManager::m_lastSendTime = now;
                    return original_send(s, buf, len, flags);
                }
                case NetworkMode::MimickingMode: {
                    // Buffer the outgoing data
                    NetworkManager::m_sendBuffer.insert(NetworkManager::m_sendBuffer.end(), buf, buf + len);

                    auto now = std::chrono::steady_clock::now();
                    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - NetworkManager::m_lastSendTime).count();

                    // Send if buffer is full enough or enough time has passed
                    if (NetworkManager::m_sendBuffer.size() >= static_cast<size_t>(NetworkManager::m_profile.avgPacketSize) || elapsedMs >= NetworkManager::m_profile.avgPacketIntervalMs) {
                        int sent = original_send(s, NetworkManager::m_sendBuffer.data(), static_cast<int>(NetworkManager::m_sendBuffer.size()), flags);
                        NetworkManager::m_sendBuffer.clear();
                        NetworkManager::m_lastSendTime = now;
                        return sent;
                    }
                    return len; // Pretend we sent the data successfully
                }
                case NetworkMode::PassThrough:
                default: {
                    return original_send(s, buf, len, flags);
                }
            }
        }

        int WSAAPI Detour_Recv(SOCKET s, char* buf, int len, int flags) {
            Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::NETWORK_PACKET_RECEIVED);
            // Recv is typically just passed through, but could also be profiled.
            return original_recv(s, buf, len, flags);
        }
        #endif

        bool NetworkManager::Install() {
            #ifndef _WIN32
            return false;
            #else
            HMODULE hModule = GetModuleHandleA(XorS("ws2_32.dll"));
            if (!hModule) return false;
            const char* sendPattern = "8B FF 55 8B EC 83 EC ?? 53 56 57 8B 7D";
            const char* recvPattern = "8B FF 55 8B EC 83 EC ?? A1 ?? ?? ?? ?? 33 C5";
            void* sendAddr = Security::SignatureScanner::FindPattern(hModule, sendPattern);
            void* recvAddr = Security::SignatureScanner::FindPattern(hModule, recvPattern);
            if (!sendAddr || !recvAddr) return false;
            auto& eventManager = EventManager::GetInstance();
            if (eventManager.Install(sendAddr, (void*)Detour_Send) && eventManager.Install(recvAddr, (void*)Detour_Recv)) {
                original_send = eventManager.GetOriginal<send_t>(sendAddr);
                original_recv = eventManager.GetOriginal<recv_t>(recvAddr);
                m_lastSendTime = std::chrono::steady_clock::now();
                return original_send && original_recv;
            }
            return false;
            #endif
        }

        void NetworkManager::Uninstall() {
            #ifndef _WIN32
            return;
            #else
            HMODULE hModule = GetModuleHandleA(XorS("ws2_32.dll"));
            if (!hModule) return;
            const char* sendPattern = "8B FF 55 8B EC 83 EC ?? 53 56 57 8B 7D";
            const char* recvPattern = "8B FF 55 8B EC 83 EC ?? A1 ?? ?? ?? ?? 33 C5";
            void* sendAddr = Security::SignatureScanner::FindPattern(hModule, sendPattern);
            void* recvAddr = Security::SignatureScanner::FindPattern(hModule, recvPattern);
            auto& eventManager = EventManager::GetInstance();
            if (sendAddr) eventManager.Uninstall(sendAddr);
            if (recvAddr) eventManager.Uninstall(recvAddr);
            #endif
        }

    }
}
