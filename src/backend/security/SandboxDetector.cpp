#include "SandboxDetector.h"
#include <Windows.h>

namespace AetherVisor {
    namespace Security {

        bool SandboxDetector::IsInSandbox() {
            if (CheckForSandboxie()) return true;
            if (CheckForVMware()) return true;
            if (CheckForVirtualBox()) return true;

            return false;
        }

        bool SandboxDetector::CheckForSandboxie() {
            if (GetModuleHandle(L"SbieDll.dll") != NULL) {
                return true;
            }
            return false;
        }

        bool SandboxDetector::CheckForVMware() {
            bool isVmware = false;
            __try {
                // VMware's I/O port backdoor.
                __asm {
                    mov eax, 'VMXh'
                    mov ebx, 0
                    mov ecx, 10
                    mov edx, 'VX'
                    in eax, dx
                    cmp ebx, 'VMXh'
                    setz isVmware
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
            return isVmware;
        }

        bool SandboxDetector::CheckForVirtualBox() {
            // Check for VirtualBox guest additions DLL
            if (GetModuleHandle(L"VBoxGuest.dll") != NULL) {
                return true;
            }
            return false;
        }

    } // namespace Security
} // namespace AetherVisor
