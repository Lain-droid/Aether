#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <chrono>
#include <queue>
#include <vector>
#include "../AIController.h"

namespace AetherVisor {
    namespace IPC {

        /**
         * @brief AI-Synchronized Named Pipe Server with Intelligent Communication
         * @details Enhanced IPC system that coordinates with AI for secure, adaptive communication
         */
        class NamedPipeServer {
        public:
            using InjectHandler = std::function<bool(const std::wstring&)>;
            using ExecuteHandler = std::function<bool(const std::string&)>;
            using ThreatHandler = std::function<void(double)>;

            enum class SecurityLevel {
                LOW,        // Basic encryption
                MEDIUM,     // Advanced encryption + obfuscation
                HIGH,       // Military-grade + AI adaptive
                CRITICAL    // Quantum-resistant + full AI coordination
            };

            struct PipeMetrics {
                uint64_t totalConnections = 0;
                uint64_t suspiciousConnections = 0;
                uint64_t encryptedMessages = 0;
                uint64_t detectionAttempts = 0;
                std::chrono::milliseconds avgResponseTime{0};
                double threatLevel = 0.0;
            };

            struct MessageEnvelope {
                uint32_t messageId;
                uint64_t timestamp;
                SecurityLevel securityLevel;
                std::vector<uint8_t> encryptedPayload;
                std::vector<uint8_t> signature;
                bool isDecoy = false;
            };

            NamedPipeServer();
            ~NamedPipeServer();

            // AI-enhanced initialization
            bool InitializeWithAI(std::shared_ptr<Backend::AIController> aiController);
            
            // Enhanced start with AI coordination
            bool Start(const std::wstring& pipe_name,
                       InjectHandler on_inject,
                       ExecuteHandler on_execute,
                       ThreatHandler on_threat = nullptr);
            void Stop();

            // AI synchronization methods
            void SyncWithAI();
            void AdaptSecurityLevel();
            void ProcessThreatIntelligence();
            
            // Advanced security features
            void SetSecurityLevel(SecurityLevel level);
            void ActivateStealthMode();
            void InjectDecoyTraffic();
            bool DetectSniffingAttempts();
            
            // Message processing
            bool SendEncryptedMessage(const std::vector<uint8_t>& data);
            std::vector<uint8_t> ReceiveDecryptedMessage();
            void ProcessMessageQueue();
            
            // Monitoring and metrics
            PipeMetrics GetMetrics() const { return m_metrics; }
            void ResetMetrics();
            bool IsCompromised() const;
            double CalculateThreatScore() const;

        private:
            void ServerThreadProc(std::wstring pipe_name);
            void SecurityMonitoringThread();
            void DecoyTrafficThread();
            
            // AI-enhanced security methods
            void AnalyzeConnectionPattern();
            void UpdateThreatAssessment();
            void GenerateDecoyMessages();
            bool ValidateClientAuthenticity();
            
            // Encryption and obfuscation
            std::vector<uint8_t> EncryptMessage(const std::vector<uint8_t>& data);
            std::vector<uint8_t> DecryptMessage(const std::vector<uint8_t>& encrypted);
            void RotateEncryptionKeys();
            void ObfuscateTraffic();

            std::thread m_thread;
            std::thread m_securityThread;
            std::thread m_decoyThread;
            std::atomic<bool> m_running{false};
            std::atomic<bool> m_stealthMode{false};
            
            InjectHandler m_on_inject;
            ExecuteHandler m_on_execute;
            ThreatHandler m_on_threat;
            
            std::shared_ptr<Backend::AIController> m_aiController;
            SecurityLevel m_currentSecurityLevel = SecurityLevel::MEDIUM;
            PipeMetrics m_metrics;
            
            std::queue<MessageEnvelope> m_incomingQueue;
            std::queue<MessageEnvelope> m_outgoingQueue;
            mutable std::mutex m_queueMutex;
            mutable std::mutex m_securityMutex;
            
            std::vector<uint8_t> m_encryptionKey;
            std::chrono::steady_clock::time_point m_lastKeyRotation;
            std::chrono::steady_clock::time_point m_lastThreatAnalysis;
            
            uint32_t m_nextMessageId = 1;
            bool m_compromised = false;

#ifdef _WIN32
            HANDLE m_pipeHandle = INVALID_HANDLE_VALUE;
#endif
        };

    } // namespace IPC
} // namespace AetherVisor

