#include "IPC.h"
#include "ipc/Steganography.h"
#include <Windows.h>
#include <thread>
#include <stdexcept>

namespace AetherVisor {
    namespace Backend {

        // Helper function to pack an IpcMessage into a StegoPacket
        IPC::StegoPacket PackMessage(const IpcMessage& msg) {
            IPC::StegoPacket packet;
            std::vector<uint8_t> payload(msg.payload.begin(), msg.payload.end());
            packet.infoHeader.height = static_cast<int32_t>(msg.type);
            packet.pixelData = payload;
            packet.fileHeader.fileSize = sizeof(packet.fileHeader) + sizeof(packet.infoHeader) + payload.size();
            packet.infoHeader.imageSize = payload.size();
            return packet;
        }

        // Helper function to unpack a StegoPacket into an IpcMessage
        IpcMessage UnpackMessage(const IPC::StegoPacket& packet) {
            IpcMessage msg;
            msg.type = static_cast<MessageType>(packet.infoHeader.height);
            if (!packet.pixelData.empty()) {
                msg.payload = std::string(packet.pixelData.begin(), packet.pixelData.end());
            }
            return msg;
        }


        void ServerLoop(HANDLE hPipe, IPC::ScriptExecutionCallback execCb, IPC::ScriptAnalysisCallback analysisCb, bool* isRunning) {
            char buffer[8192]; // Increased buffer size
            DWORD bytesRead;

            while (*isRunning) {
                if (ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)) {
                    while (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
                        std::vector<uint8_t> receivedData(buffer, buffer + bytesRead);
                        IPC::StegoPacket packet = IPC::StegoPacket::Deserialize(receivedData);
                        IpcMessage msg = UnpackMessage(packet);

                        switch (msg.type) {
                            case MessageType::ExecuteScript:
                                if (execCb) execCb(msg.payload);
                                break;
                            case MessageType::AnalyzeScriptRequest:
                                if (analysisCb) analysisCb(msg.payload);
                                break;
                            case MessageType::Shutdown:
                                *isRunning = false;
                                break;
                            default:
                                break;
                        }
                    }
                    DisconnectNamedPipe(hPipe);
                }
            }
        }

        IPC::IPC() {}
        IPC::~IPC() { StopServer(); }

        bool IPC::StartServer(ScriptExecutionCallback execCallback, ScriptAnalysisCallback analysisCallback) {
            LPCSTR pipeName = "\\\\.\\pipe\\AetherVisor_Session_Pipe";
            m_pipeHandle = CreateNamedPipeA(
                pipeName, PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                1, 8192, 8192, 0, NULL);

            if (m_pipeHandle == INVALID_HANDLE_VALUE) return false;

            m_isRunning = true;
            std::thread serverThread(ServerLoop, m_pipeHandle, execCallback, analysisCallback, &m_isRunning);
            serverThread.detach();
            return true;
        }

        bool IPC::SendMessageToFrontend(const IpcMessage& message) {
            if (m_pipeHandle == INVALID_HANDLE_VALUE) return false;
            IPC::StegoPacket packet = PackMessage(message);
            std::vector<uint8_t> buffer = packet.Serialize();
            DWORD bytesWritten;
            return WriteFile(m_pipeHandle, buffer.data(), buffer.size(), &bytesWritten, NULL) && (bytesWritten == buffer.size());
        }

        void IPC::StopServer() {
            m_isRunning = false;
            if (m_pipeHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(m_pipeHandle);
                m_pipeHandle = INVALID_HANDLE_VALUE;
            }
        }
    }
}
