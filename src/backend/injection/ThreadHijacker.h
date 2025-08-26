#pragma once

#include <Windows.h>
#include <vector>

namespace AetherVisor {
    namespace Injection {

        class ThreadHijacker {
        public:
            ThreadHijacker();

            // Injects shellcode into the target process by hijacking a thread.
            bool Inject(HANDLE hProcess, const std::vector<BYTE>& shellcode);

        private:
            // Helper to get the ID of the main thread of a process.
            DWORD GetMainThreadId(DWORD processId);
        };

    } // namespace Injection
} // namespace AetherVisor
