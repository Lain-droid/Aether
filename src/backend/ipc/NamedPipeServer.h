#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <string>
#include <thread>
#include <atomic>
#include <functional>

namespace AetherVisor {
    namespace IPC {

        class NamedPipeServer {
        public:
            using InjectHandler = std::function<bool(const std::wstring&)>;
            using ExecuteHandler = std::function<bool(const std::string&)>;

            NamedPipeServer();
            ~NamedPipeServer();

            bool Start(const std::wstring& pipe_name,
                       InjectHandler on_inject,
                       ExecuteHandler on_execute);
            void Stop();

        private:
            void ServerThreadProc(std::wstring pipe_name);

            std::thread m_thread;
            std::atomic<bool> m_running{false};
            InjectHandler m_on_inject;
            ExecuteHandler m_on_execute;

#ifdef _WIN32
            HANDLE m_pipeHandle = INVALID_HANDLE_VALUE;
#endif
        };

    } // namespace IPC
} // namespace AetherVisor

