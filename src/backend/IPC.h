#pragma once

#include <string>
#include <functional>

namespace AetherVisor {
    namespace Backend {

        // Defines the types of messages that can be sent over IPC
        enum class MessageType {
            // Frontend -> Backend
            ExecuteScript,
            AnalyzeScriptRequest,
            Shutdown,

            // Backend -> Frontend
            AnalyzeScriptResponse,
            ConsoleOutput,
            StatusUpdate
        };

        // Represents a message packet for IPC
        struct IpcMessage {
            MessageType type;
            std::string payload;
        };

        class IPC {
        public:
            // Callback for when a script execution is requested from the frontend
            using ScriptExecutionCallback = std::function<void(const std::string& script)>;
            using ScriptAnalysisCallback = std::function<void(const std::string& script)>;

            IPC();
            ~IPC();

            // Starts the IPC server to listen for frontend connections
            bool StartServer(ScriptExecutionCallback execCallback, ScriptAnalysisCallback analysisCallback);

            // Sends a message (e.g., console output) to the frontend
            bool SendMessageToFrontend(const IpcMessage& message);

            // Stops the IPC server
            void StopServer();

        private:
            // Internal implementation details (e.g., named pipe handle)
            void* m_pipeHandle = nullptr;
            bool m_isRunning = false;
        };

    } // namespace Backend
} // namespace AetherVisor
