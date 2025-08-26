#include "SandboxDetector.h"
#include <Windows.h>
#include <vector>
#include <string>

namespace AetherVisor {
    namespace Security {

        bool SandboxDetector::IsInSandbox() {
            if (CheckForSandboxie()) return true;
            if (CheckForVMware()) return true;
            if (CheckForVirtualBox()) return true;
            if (CheckUsername()) return true;
            if (CheckUptime()) return true;

            return false;
        }

        bool SandboxDetector::CheckForSandboxie() {
            if (GetModuleHandle(L"SbieDll.dll") != NULL) return true;
            return false;
        }

        bool SandboxDetector::CheckForVMware() {
            bool isVmware = false;
            __try {
                __asm { mov eax, 'VMXh'; mov ebx, 0; mov ecx, 10; mov edx, 'VX'; in eax, dx; cmp ebx, 'VMXh'; setz isVmware; }
            } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
            return isVmware;
        }

        bool SandboxDetector::CheckForVirtualBox() {
            if (GetModuleHandle(L"VBoxGuest.dll") != NULL) return true;
            return false;
        }

        bool SandboxDetector::CheckUsername() {
            wchar_t username[257];
            DWORD username_len = 257;
            if (GetUserNameW(username, &username_len)) {
                std::wstring user(username);
                std::vector<std::wstring> bad_users = { L"CurrentUser", L"Sandbox", L"Emily", L"test", L"user", L"admin" };
                for (const auto& bad_user : bad_users) {
                    if (user == bad_user) return true;
                }
            }
            return false;
        }

        bool SandboxDetector::CheckUptime() {
            // Check if system uptime is less than 10 minutes
            if (GetTickCount64() < 600000) {
                return true;
            }
            return false;
        }

    } // namespace Security
} // namespace AetherVisor
