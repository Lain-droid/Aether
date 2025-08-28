#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <atomic>
#include <functional>
#include <thread>
#include "SecurityTypes.h"

namespace Aether::IPC {

    // Forward declarations
    class AI::AIController;
    class EncryptionEngine;
    class MemoryManager;
    class AntiSniffingProtector;

    /**
     * @brief Encrypted Shared Memory IPC System
     * @details Provides secure, encrypted communication between AI compiler and game script
     *          using shared memory with advanced anti-sniffing protection against Hyperion
     */
    class EncryptedSharedMemoryIPC {
    public:
        enum class ChannelType {
            CommandChannel,      // Commands from frontend to backend
            ResponseChannel,     // Responses from backend to frontend
            ScriptChannel,       // Script data transmission
            AIDataChannel,       // AI model and learning data
            DebugChannel,        // Debug and logging information
            EmergencyChannel     // Emergency shutdown and security alerts
        };

        enum class MessageType {
            Handshake,
            Command,
            Response,
            ScriptData,
            AIData,
            HeartBeat,
            SecurityAlert,
            Emergency,
            Termination
        };

        enum class EncryptionLevel {
            Basic = 1,           // XOR encryption
            Standard = 2,        // AES-128
            Advanced = 3,        // AES-256
            Military = 4,        // AES-256 + RSA
            Quantum = 5,         // Post-quantum cryptography
            AIAdaptive = 6       // AI-selected encryption
        };

        struct ChannelConfig {
            ChannelType type;
            std::string channelName;
            size_t bufferSize;
            EncryptionLevel encryptionLevel;
            bool enableCompression;
            bool enableAntiSniffing;
            std::chrono::milliseconds timeout;
            uint32_t maxRetries;
        };

        struct MessageHeader {
            uint32_t magic;              // Magic signature
            MessageType type;            // Message type
            uint32_t sequenceNumber;     // Sequence number for ordering
            uint32_t totalSize;          // Total message size
            uint32_t encryptedSize;      // Encrypted payload size
            uint32_t checksum;           // Integrity checksum
            uint64_t timestamp;          // Message timestamp
            uint32_t processId;          // Sender process ID
            uint32_t threadId;           // Sender thread ID
            std::array<uint8_t, 16> nonce; // Encryption nonce
            std::array<uint8_t, 32> hmac;  // Message authentication code
        };

        struct IPCMetrics {
            uint64_t messagesSent;
            uint64_t messagesReceived;
            uint64_t bytesTransferred;
            uint64_t encryptionOperations;
            uint64_t decryptionOperations;
            uint64_t sniffingAttempts;
            uint64_t integrityFailures;
            std::chrono::milliseconds totalLatency;
            std::chrono::steady_clock::time_point lastActivity;
        };

    private:
        std::unique_ptr<AI::AIController> m_aiController;
        std::unique_ptr<EncryptionEngine> m_encryptionEngine;
        std::unique_ptr<MemoryManager> m_memoryManager;
        std::unique_ptr<AntiSniffingProtector> m_antiSniffingProtector;

        std::unordered_map<ChannelType, ChannelConfig> m_channelConfigs;
        std::unordered_map<ChannelType, HANDLE> m_sharedMemoryHandles;
        std::unordered_map<ChannelType, LPVOID> m_sharedMemoryViews;
        std::unordered_map<ChannelType, HANDLE> m_mutexHandles;
        std::unordered_map<ChannelType, HANDLE> m_eventHandles;

        IPCMetrics m_metrics;
        std::atomic<bool> m_isInitialized;
        std::atomic<bool> m_isShutdown;

        mutable std::mutex m_channelMutex;
        mutable std::mutex m_metricsMutex;

        // Anti-sniffing measures
        std::thread m_antiSniffingThread;
        std::atomic<bool> m_antiSniffingActive;
        std::vector<uint8_t> m_obfuscationKey;
        std::chrono::steady_clock::time_point m_lastKeyRotation;

        // Configuration
        struct IPCConfig {
            EncryptionLevel defaultEncryptionLevel = EncryptionLevel::Advanced;
            bool enableAntiSniffing = true;
            bool enableCompression = true;
            bool enableHeartBeat = true;
            std::chrono::minutes keyRotationInterval{10};
            std::chrono::seconds heartBeatInterval{30};
            size_t defaultBufferSize = 1024 * 1024; // 1MB
            uint32_t maxChannels = 10;
        } m_config;

    public:
        explicit EncryptedSharedMemoryIPC(std::shared_ptr<AI::AIController> aiController);
        ~EncryptedSharedMemoryIPC();

        // Initialization and lifecycle
        bool Initialize();
        void Shutdown();
        bool IsInitialized() const { return m_isInitialized.load(); }

        // Channel management
        bool CreateChannel(const ChannelConfig& config);
        bool DestroyChannel(ChannelType type);
        bool ConnectToChannel(ChannelType type, DWORD processId);
        bool DisconnectFromChannel(ChannelType type);

        // Message operations
        bool SendMessage(ChannelType channel, MessageType type, const std::vector<uint8_t>& data);
        std::vector<uint8_t> ReceiveMessage(ChannelType channel, MessageType& type);
        bool SendCommand(const std::string& command, const std::vector<uint8_t>& parameters);
        std::vector<uint8_t> ReceiveResponse(std::chrono::milliseconds timeout);

        // Specialized communication methods
        bool SendScriptData(const std::string& script);
        std::string ReceiveScriptData();
        bool SendAIData(const std::vector<uint8_t>& aiData);
        std::vector<uint8_t> ReceiveAIData();

        // Security and anti-sniffing
        bool DetectSniffingAttempt();
        void ActivateAntiSniffingMeasures();
        bool RotateEncryptionKeys();
        bool ValidateChannelIntegrity(ChannelType channel);

        // AI integration
        EncryptionLevel SelectOptimalEncryption(ChannelType channel, size_t dataSize);
        void AdaptToSniffingAttempt();
        bool OptimizeChannelPerformance();

        // Utility and monitoring
        IPCMetrics GetMetrics() const;
        void ResetMetrics();
        bool PerformHandshake(ChannelType channel);
        bool SendHeartBeat();
        std::vector<ChannelType> GetActiveChannels();

        // Configuration
        void SetEncryptionLevel(EncryptionLevel level);
        void EnableAntiSniffing(bool enable) { m_config.enableAntiSniffing = enable; }
        void SetKeyRotationInterval(std::chrono::minutes interval);

    private:
        // Core IPC operations
        bool CreateSharedMemory(ChannelType channel, size_t size);
        bool MapSharedMemory(ChannelType channel);
        bool UnmapSharedMemory(ChannelType channel);
        bool CreateSynchronizationObjects(ChannelType channel);

        // Message serialization
        std::vector<uint8_t> SerializeMessage(MessageType type, const std::vector<uint8_t>& data);
        bool DeserializeMessage(const std::vector<uint8_t>& serializedData, 
                               MessageType& type, std::vector<uint8_t>& data);
        MessageHeader CreateMessageHeader(MessageType type, uint32_t dataSize);
        bool ValidateMessageHeader(const MessageHeader& header);

        // Encryption and security
        std::vector<uint8_t> EncryptData(const std::vector<uint8_t>& data, ChannelType channel);
        std::vector<uint8_t> DecryptData(const std::vector<uint8_t>& encryptedData, ChannelType channel);
        uint32_t CalculateChecksum(const std::vector<uint8_t>& data);
        std::array<uint8_t, 32> CalculateHMAC(const std::vector<uint8_t>& data, ChannelType channel);

        // Anti-sniffing implementations
        void AntiSniffingLoop();
        bool DetectMemoryScanning();
        bool DetectProcessAttachment();
        bool DetectAPIHooking();
        void ObfuscateMemoryContent();
        void InsertDecoyData();

        // Synchronization helpers
        bool AcquireChannelLock(ChannelType channel, std::chrono::milliseconds timeout);
        void ReleaseChannelLock(ChannelType channel);
        bool WaitForChannelEvent(ChannelType channel, std::chrono::milliseconds timeout);
        void SignalChannelEvent(ChannelType channel);

        // Utility methods
        std::string GenerateChannelName(ChannelType type);
        std::string GenerateMutexName(ChannelType type);
        std::string GenerateEventName(ChannelType type);
        bool IsChannelActive(ChannelType channel);
        size_t GetChannelBufferSize(ChannelType channel);
    };

    /**
     * @brief Encryption Engine for IPC message protection
     */
    class EncryptionEngine {
    public:
        enum class CipherType {
            XOR,
            AES128,
            AES256,
            ChaCha20,
            RSA2048,
            RSA4096,
            Hybrid
        };

        struct EncryptionContext {
            CipherType cipher;
            std::vector<uint8_t> key;
            std::vector<uint8_t> iv;
            std::vector<uint8_t> salt;
            uint32_t rounds;
        };

    private:
        std::unordered_map<EncryptedSharedMemoryIPC::ChannelType, EncryptionContext> m_contexts;
        std::shared_ptr<AI::AIController> m_aiController;

    public:
        explicit EncryptionEngine(std::shared_ptr<AI::AIController> aiController);
        bool Initialize();
        bool SetupChannel(EncryptedSharedMemoryIPC::ChannelType channel, 
                         EncryptedSharedMemoryIPC::EncryptionLevel level);
        std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& data, 
                                   EncryptedSharedMemoryIPC::ChannelType channel);
        std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& encryptedData, 
                                   EncryptedSharedMemoryIPC::ChannelType channel);
        bool RotateKey(EncryptedSharedMemoryIPC::ChannelType channel);
    };

    /**
     * @brief Memory Manager for shared memory allocation and protection
     */
    class MemoryManager {
    public:
        struct MemoryRegion {
            LPVOID address;
            size_t size;
            DWORD protection;
            bool isEncrypted;
            bool isObfuscated;
            std::chrono::steady_clock::time_point lastAccess;
        };

    private:
        std::unordered_map<EncryptedSharedMemoryIPC::ChannelType, MemoryRegion> m_regions;
        std::shared_ptr<AI::AIController> m_aiController;

    public:
        explicit MemoryManager(std::shared_ptr<AI::AIController> aiController);
        bool AllocateRegion(EncryptedSharedMemoryIPC::ChannelType channel, size_t size);
        bool DeallocateRegion(EncryptedSharedMemoryIPC::ChannelType channel);
        bool ProtectRegion(EncryptedSharedMemoryIPC::ChannelType channel, DWORD protection);
        bool ObfuscateRegion(EncryptedSharedMemoryIPC::ChannelType channel);
        MemoryRegion GetRegionInfo(EncryptedSharedMemoryIPC::ChannelType channel);
    };

    /**
     * @brief Anti-Sniffing Protector for detecting and preventing IPC monitoring
     */
    class AntiSniffingProtector {
    public:
        enum class SniffingThreat {
            MemoryScanning,
            ProcessAttachment,
            APIHooking,
            NetworkMonitoring,
            RegistryMonitoring,
            FileSystemMonitoring
        };

        struct ThreatEvent {
            SniffingThreat threat;
            std::string description;
            std::chrono::steady_clock::time_point timestamp;
            bool wasMitigated;
        };

    private:
        std::vector<ThreatEvent> m_threatEvents;
        std::shared_ptr<AI::AIController> m_aiController;
        std::atomic<bool> m_isActive;

    public:
        explicit AntiSniffingProtector(std::shared_ptr<AI::AIController> aiController);
        bool Initialize();
        bool DetectThreat(SniffingThreat threat);
        bool MitigateThreat(SniffingThreat threat);
        void ActivateProtection();
        void DeactivateProtection();
        std::vector<ThreatEvent> GetThreatHistory() const;
    };

} // namespace Aether::IPC