#include "SandboxDetector.h"
#include <Windows.h>
#include <vector>
#include <string>
#include <Iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")

namespace AetherVisor {
    namespace Security {

        bool SandboxDetector::IsInSandbox() {
            if (CheckForSandboxie()) return true;
            if (CheckForVMware()) return true;
            if (CheckForVirtualBox()) return true;
            if (CheckUsername()) return true;
            if (CheckUptime()) return true;
            if (CheckMACAddress()) return true;

            return false;
        }

        bool SandboxDetector::CheckForSandboxie() { if (GetModuleHandle(L"SbieDll.dll") != NULL) return true; return false; }
        bool SandboxDetector::CheckForVMware() { bool isVmware = false; __try { __asm { mov eax, 'VMXh'; mov ebx, 0; mov ecx, 10; mov edx, 'VX'; in eax, dx; cmp ebx, 'VMXh'; setz isVmware; } } __except (EXCEPTION_EXECUTE_HANDLER) { return false; } return isVmware; }
        bool SandboxDetector::CheckForVirtualBox() { if (GetModuleHandle(L"VBoxGuest.dll") != NULL) return true; return false; }

        bool SandboxDetector::CheckUsername() {
            wchar_t username[257];
            DWORD username_len = 257;
            if (GetUserNameW(username, &username_len)) {
                std::wstring user(username);
                std::vector<std::wstring> bad_users = { L"CurrentUser", L"Sandbox", L"Emily", L"test", L"user", L"admin" };
                for (const auto& bad_user : bad_users) { if (user == bad_user) return true; }
            }
            return false;
        }

        bool SandboxDetector::CheckUptime() {
            if (GetTickCount64() < 600000) return true; // Less than 10 minutes
            return false;
        }

        bool SandboxDetector::CheckMACAddress() {
            std::vector<std::string> bad_mac_prefixes = {
                "00:05:69", // VMware
                "00:0C:29", // VMware
                "00:1C:14", // VMware
                "00:50:56", // VMware
                "08:00:27"  // VirtualBox
            };

            ULONG bufferSize = sizeof(IP_ADAPTER_INFO);
            std::vector<BYTE> buffer(bufferSize);
            PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)buffer.data();

            if (GetAdaptersInfo(pAdapterInfo, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
                buffer.resize(bufferSize);
                pAdapterInfo = (PIP_ADAPTER_INFO)buffer.data();
            }

            if (GetAdaptersInfo(pAdapterInfo, &bufferSize) == NO_ERROR) {
                while (pAdapterInfo) {
                    char mac_addr[18];
                    sprintf_s(mac_addr, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                        pAdapterInfo->Address[0], pAdapterInfo->Address[1], pAdapterInfo->Address[2],
                        pAdapterInfo->Address[3], pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
                    std::string mac_str(mac_addr);

                    for (const auto& prefix : bad_mac_prefixes) {
                        if (mac_str.rfind(prefix, 0) == 0) return true; // Starts with bad prefix
                    }
                    pAdapterInfo = pAdapterInfo->Next;
                }
            }
            return false;
        }

    } // namespace Security
} // namespace AetherVisor
