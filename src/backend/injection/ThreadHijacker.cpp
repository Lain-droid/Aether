#include "ThreadHijacker.h"
#include <tlhelp32.h>

namespace AetherVisor {
    namespace Injection {

        ThreadHijacker::ThreadHijacker() {}

        DWORD ThreadHijacker::GetMainThreadId(DWORD processId) {
            THREADENTRY32 te32;
            te32.dwSize = sizeof(THREADENTRY32);
            HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            if (hThreadSnap == INVALID_HANDLE_VALUE) return 0;

            Thread32First(hThreadSnap, &te32);
            do {
                if (te32.th32OwnerProcessID == processId) {
                    CloseHandle(hThreadSnap);
                    return te32.th32ThreadID;
                }
            } while (Thread32Next(hThreadSnap, &te32));

            CloseHandle(hThreadSnap);
            return 0;
        }

        bool ThreadHijacker::Inject(HANDLE hProcess, const std::vector<BYTE>& shellcode) {
            if (shellcode.empty()) return false;

            DWORD threadId = GetMainThreadId(GetProcessId(hProcess));
            if (threadId == 0) return false;

            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadId);
            if (!hThread) return false;

            if (SuspendThread(hThread) == (DWORD)-1) {
                CloseHandle(hThread);
                return false;
            }

            LPVOID pRemoteShellcode = VirtualAllocEx(hProcess, NULL, shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!pRemoteShellcode) {
                ResumeThread(hThread);
                CloseHandle(hThread);
                return false;
            }

            if (!WriteProcessMemory(hProcess, pRemoteShellcode, shellcode.data(), shellcode.size(), NULL)) {
                VirtualFreeEx(hProcess, pRemoteShellcode, 0, MEM_RELEASE);
                ResumeThread(hThread);
                CloseHandle(hThread);
                return false;
            }

            CONTEXT context;
            context.ContextFlags = CONTEXT_FULL;
            if (!GetThreadContext(hThread, &context)) {
                 VirtualFreeEx(hProcess, pRemoteShellcode, 0, MEM_RELEASE);
                ResumeThread(hThread);
                CloseHandle(hThread);
                return false;
            }

            // Point the instruction pointer to our shellcode
            #ifdef _WIN64
            context.Rip = (DWORD64)pRemoteShellcode;
            #else
            context.Eip = (DWORD)pRemoteShellcode;
            #endif

            if (!SetThreadContext(hThread, &context)) {
                VirtualFreeEx(hProcess, pRemoteShellcode, 0, MEM_RELEASE);
                ResumeThread(hThread);
                CloseHandle(hThread);
                return false;
            }

            ResumeThread(hThread);
            CloseHandle(hThread);
            return true;
        }

    } // namespace Injection
} // namespace AetherVisor
