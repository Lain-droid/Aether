#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "IPC.h"
#include "ipc/Steganography.h"
#include <thread>
#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace AetherVisor {
	namespace Backend {

		// Helper function to pack an IpcMessage into a StegoPacket
		::AetherVisor::IPC::StegoPacket PackMessage(const IpcMessage& msg) {
			::AetherVisor::IPC::StegoPacket packet;
			std::vector<uint8_t> payload(msg.payload.begin(), msg.payload.end());

			packet.infoHeader.width = payload.size();
			packet.infoHeader.height = static_cast<int32_t>(msg.type);
			packet.pixelData = payload;

			packet.fileHeader.fileSize = sizeof(packet.fileHeader) + sizeof(packet.infoHeader) + payload.size();
			packet.infoHeader.imageSize = payload.size();

			return packet;
		}

		// Helper function to unpack a StegoPacket into an IpcMessage
		IpcMessage UnpackMessage(const ::AetherVisor::IPC::StegoPacket& packet) {
			IpcMessage msg;
			msg.type = static_cast<MessageType>(packet.infoHeader.height);
			if (!packet.pixelData.empty()) {
				msg.payload = std::string(packet.pixelData.begin(), packet.pixelData.end());
			}
			return msg;
		}


		#ifdef _WIN32
		void ServerLoop(HANDLE hPipe, IPC::ScriptExecutionCallback callback, bool* isRunning) {
			char buffer[4096];
			DWORD bytesRead;

			while (*isRunning) {
				if (ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)) {
					while (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
						std::vector<uint8_t> receivedData(buffer, buffer + bytesRead);
						::AetherVisor::IPC::StegoPacket packet = ::AetherVisor::IPC::StegoPacket::Deserialize(receivedData);
						IpcMessage msg = UnpackMessage(packet);

						// For now, we only handle script execution from frontend
						if (msg.type == MessageType::ExecuteScript) {
							callback(msg.payload);
						}
					}
					DisconnectNamedPipe(hPipe);
				}
			}
		}
		#endif

		IPC::IPC() {}

		IPC::~IPC() {
			StopServer();
		}

		bool IPC::StartServer(ScriptExecutionCallback callback) {
			#ifndef _WIN32
			(void)callback;
			return false; // Not supported on non-Windows in this build
			#else
			LPCSTR pipeName = "\\\\.\\pipe\\AetherVisor_Session_Pipe";
			m_pipeHandle = CreateNamedPipeA(
				pipeName,
				PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
				1, 4096, 4096, 0, NULL);
			if (m_pipeHandle == INVALID_HANDLE_VALUE) {
				return false;
			}
			m_isRunning = true;
			std::thread serverThread(ServerLoop, m_pipeHandle, callback, &m_isRunning);
			serverThread.detach();
			return true;
			#endif
		}

		bool IPC::SendMessageToFrontend(const IpcMessage& message) {
			#ifndef _WIN32
			(void)message;
			return false;
			#else
			if (m_pipeHandle == INVALID_HANDLE_VALUE) return false;

			::AetherVisor::IPC::StegoPacket packet = PackMessage(message);
			std::vector<uint8_t> buffer = packet.Serialize();

			DWORD bytesWritten;
			if (WriteFile(m_pipeHandle, buffer.data(), buffer.size(), &bytesWritten, NULL)) {
				return bytesWritten == buffer.size();
			}
			return false;
			#endif
		}

		void IPC::StopServer() {
			m_isRunning = false;
			#ifdef _WIN32
			if (m_pipeHandle != INVALID_HANDLE_VALUE) {
				CloseHandle(m_pipeHandle);
				m_pipeHandle = INVALID_HANDLE_VALUE;
			}
			#else
			m_pipeHandle = nullptr;
			#endif
		}

	}
}
