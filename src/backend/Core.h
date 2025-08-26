#pragma once

#include <Windows.h>
#include <string>

namespace AetherVisor {
    namespace Backend {

        class Core {
        public:
            // Singleton instance accessor
            static Core& GetInstance();

            // Initializes the backend services
            bool Initialize();

            // Starts the injection process into the target
            bool Inject(const std::wstring& processName);

            // Triggers a full cleanup of all injected components
            void Cleanup();

#include "IPC.h"

        private:
            Core();
            ~Core() = default;
            Core(const Core&) = delete;
            Core& operator=(const Core&) = delete;

            bool m_initialized = false;
            HANDLE m_targetProcess = nullptr;
            IPC m_ipc;
        };

    } // namespace Backend
} // namespace AetherVisor
