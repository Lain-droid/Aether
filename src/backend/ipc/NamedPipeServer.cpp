#include "NamedPipeServer.h"
#include <vector>

namespace AetherVisor {
    namespace IPC {

        static bool ReadMessage(HANDLE pipe, std::vector<char>& buffer) {
            uint32_t size = 0;
            DWORD read = 0;
            if (!ReadFile(pipe, &size, sizeof(size), &read, nullptr) || read != sizeof(size)) return false;
            if (size == 0 || size > 1u << 20) return false; // limit 1MB
            buffer.resize(size);
            return ReadFile(pipe, buffer.data(), size, &read, nullptr) && read == size;
        }

        static bool WriteMessage(HANDLE pipe, const std::vector<char>& buffer) {
            uint32_t size = static_cast<uint32_t>(buffer.size());
            DWORD written = 0;
            if (!WriteFile(pipe, &size, sizeof(size), &written, nullptr) || written != sizeof(size)) return false;
            if (size == 0) return true;
            return WriteFile(pipe, buffer.data(), size, &written, nullptr) && written == size;
        }

        NamedPipeServer::NamedPipeServer() {}
        NamedPipeServer::~NamedPipeServer() { Stop(); }

        bool NamedPipeServer::Start(const std::wstring& pipe_name,
                                    InjectHandler on_inject,
                                    ExecuteHandler on_execute) {
            if (m_running.load()) return false;
            m_on_inject = std::move(on_inject);
            m_on_execute = std::move(on_execute);
            m_running = true;
            m_thread = std::thread(&NamedPipeServer::ServerThreadProc, this, pipe_name);
            return true;
        }

        void NamedPipeServer::Stop() {
            if (!m_running.exchange(false)) return;
#ifdef _WIN32
            if (m_pipeHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(m_pipeHandle);
                m_pipeHandle = INVALID_HANDLE_VALUE;
            }
#endif
            if (m_thread.joinable()) m_thread.join();
        }

        void NamedPipeServer::ServerThreadProc(std::wstring pipe_name) {
#ifdef _WIN32
            while (m_running.load()) {
                m_pipeHandle = CreateNamedPipeW((L"\\\\.\\pipe\\" + pipe_name).c_str(),
                                               PIPE_ACCESS_DUPLEX,
                                               PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                               1, 1 << 16, 1 << 16, 0, nullptr);
                if (m_pipeHandle == INVALID_HANDLE_VALUE) return;
                BOOL connected = ConnectNamedPipe(m_pipeHandle, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
                if (!connected) {
                    CloseHandle(m_pipeHandle);
                    m_pipeHandle = INVALID_HANDLE_VALUE;
                    continue;
                }

                std::vector<char> msg;
                while (m_running.load() && ReadMessage(m_pipeHandle, msg)) {
                    // Simple protocol: first byte = opcode (1=Inject, 2=Execute), payload = utf8/utf16le
                    if (msg.empty()) break;
                    uint8_t op = static_cast<uint8_t>(msg[0]);
                    bool ok = false;
                    std::string outMsg;
                    if (op == 1 && m_on_inject) {
                        // payload is utf16le wchar string after the first byte
                        if (msg.size() >= 2) {
                            const wchar_t* w = reinterpret_cast<const wchar_t*>(msg.data() + 1);
                            size_t len_bytes = msg.size() - 1;
                            size_t len_wchars = len_bytes / sizeof(wchar_t);
                            std::wstring proc(w, w + len_wchars);
                            ok = m_on_inject(proc);
                            outMsg = ok ? "OK: Inject " : "ERR: Inject ";
                            outMsg += std::string(proc.begin(), proc.end());
                        }
                    } else if (op == 2 && m_on_execute) {
                        if (msg.size() > 1) {
                            std::string script(msg.begin() + 1, msg.end());
                            ok = m_on_execute(script);
                            outMsg = ok ? "OK: Execute" : "ERR: Execute";
                        }
                    } else if (op == 3) {
                        // AI sensitivity config (double)
                        if (msg.size() >= 1 + sizeof(double)) {
                            double value = 0.5;
                            std::memcpy(&value, msg.data() + 1, sizeof(double));
                            // store globally via handler if desired later; acknowledge
                            ok = true;
                            outMsg = "OK: Config";
                        }
                    } else if (op == 4) {
                        // Start bypass modules (user-mode simulation)
                        ok = true;
                        outMsg = "OK: Bypass started";
                    }
                    if (outMsg.empty()) {
                        std::vector<char> resp(1, ok ? 1 : 0);
                        if (!WriteMessage(m_pipeHandle, resp)) break;
                    } else {
                        std::vector<char> resp(outMsg.begin(), outMsg.end());
                        if (!WriteMessage(m_pipeHandle, resp)) break;
                    }
                }

                DisconnectNamedPipe(m_pipeHandle);
                CloseHandle(m_pipeHandle);
                m_pipeHandle = INVALID_HANDLE_VALUE;
            }
#else
            (void)pipe_name;
            while (m_running.load()) { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
#endif
        }

    } // namespace IPC
} // namespace AetherVisor

