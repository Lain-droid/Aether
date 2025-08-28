#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <random>
#include <chrono>
#include <mutex>
#include <functional>
#include "SecurityTypes.h"

namespace Aether::Security {

    // Forward declarations
    class AIController;
    class SMBIOSSpoofer;
    class DiskSerialSpoofer;
    class GPUIdentityMasker;
    class NetworkAdapterSpoofer;

    /**
     * @brief Comprehensive Hardware Fingerprint Spoofing System
     * @details Provides dynamic hardware identity spoofing to prevent HWID bans
     *          and tracking by Roblox/Hyperion anti-cheat systems
     */
    class HardwareFingerprintSpoofer {
    public:
        enum class FingerprintComponent {
            SMBIOS,
            DiskSerial,
            GPUID,
            NetworkMAC,
            ProcessorID,
            MotherboardSerial,
            RAMConfiguration,
            SystemUUID,
            RegistryFingerprint,
            WMIFingerprint
        };

        enum class SpoofingLevel {
            Light = 1,      // Basic spoofing
            Medium = 2,     // Moderate spoofing with some persistence
            Heavy = 3,      // Comprehensive spoofing
            Extreme = 4,    // Maximum obfuscation (may affect stability)
            AIAdaptive = 5  // AI-guided adaptive spoofing
        };

        struct HardwareProfile {
            std::string computerName;
            std::string motherboardSerial;
            std::string biosSerial;
            std::string processorId;
            std::vector<std::string> diskSerials;
            std::string gpuDeviceId;
            std::vector<std::string> networkMACs;
            std::string systemUuid;
            std::unordered_map<std::string, std::string> registryEntries;
            std::chrono::steady_clock::time_point creationTime;
            bool isActive;
        };

        struct SpoofingSession {
            uint64_t sessionId;
            HardwareProfile originalProfile;
            HardwareProfile spoofedProfile;
            SpoofingLevel level;
            std::vector<FingerprintComponent> spoofedComponents;
            std::chrono::steady_clock::time_point startTime;
            std::chrono::steady_clock::time_point lastRefresh;
            bool isPersistent;
        };

        struct DetectionMetrics {
            uint32_t fingerprintingAttempts;
            uint32_t successfulSpoofs;
            uint32_t detectionEvasions;
            uint32_t profileChanges;
            std::chrono::milliseconds averageSpoofTime;
            std::unordered_map<FingerprintComponent, uint32_t> componentAccessCounts;
        };

    private:
        std::unique_ptr<AI::AIController> m_aiController;
        std::unique_ptr<SMBIOSSpoofer> m_smbiosSpoofer;
        std::unique_ptr<DiskSerialSpoofer> m_diskSpoofer;
        std::unique_ptr<GPUIdentityMasker> m_gpuMasker;
        std::unique_ptr<NetworkAdapterSpoofer> m_networkSpoofer;

        SpoofingSession m_currentSession;
        std::vector<HardwareProfile> m_profileDatabase;
        DetectionMetrics m_metrics;

        mutable std::mutex m_sessionMutex;
        mutable std::mutex m_profileMutex;

        std::mt19937_64 m_randomEngine;
        std::uniform_int_distribution<uint32_t> m_randomDistribution;

        // Configuration
        struct SpooferConfig {
            SpoofingLevel defaultLevel = SpoofingLevel::Medium;
            std::chrono::minutes profileRefreshInterval{15};
            bool enableAIGuidedSpoofing = true;
            bool enablePersistentSpoofing = false;
            bool enableRealTimeAdaptation = true;
            uint32_t maxProfileDatabase = 100;
            std::vector<FingerprintComponent> enabledComponents;
        } m_config;

    public:
        explicit HardwareFingerprintSpoofer(std::shared_ptr<AI::AIController> aiController);
        ~HardwareFingerprintSpoofer();

        // Initialization and lifecycle
        bool Initialize();
        void Shutdown();
        bool StartSpoofingSession(SpoofingLevel level = SpoofingLevel::Medium);
        bool StopSpoofingSession();
        bool IsActive() const;

        // Hardware profile management
        HardwareProfile GenerateRandomProfile();
        HardwareProfile GenerateAIGuidedProfile();
        bool LoadProfile(const HardwareProfile& profile);
        bool SaveProfile(const HardwareProfile& profile);
        std::vector<HardwareProfile> GetProfileDatabase() const;

        // Component-specific spoofing
        bool SpoofSMBIOS(const std::string& motherboardSerial, const std::string& biosSerial);
        bool SpoofDiskSerials(const std::vector<std::string>& newSerials);
        bool SpoofGPUIdentity(const std::string& newDeviceId, const std::string& newVendorId);
        bool SpoofNetworkMACs(const std::vector<std::string>& newMACs);
        bool SpoofProcessorID(const std::string& newProcessorId);
        bool SpoofSystemUUID(const std::string& newUuid);

        // Registry and WMI spoofing
        bool SpoofRegistryEntries(const std::unordered_map<std::string, std::string>& entries);
        bool SpoofWMIQueries();
        bool InstallWMIHooks();
        bool RemoveWMIHooks();

        // Dynamic adaptation
        void RefreshFingerprint();
        void AdaptToDetectionAttempt();
        void UpdateProfileBasedOnUsage();

        // Anti-detection measures
        bool DetectFingerprintingAttempt();
        void ActivateAntiFingerprinting();
        bool HookFingerprintingAPIs();
        bool UnhookFingerprintingAPIs();

        // AI integration
        void FeedDetectionDataToAI(FingerprintComponent component, bool detected);
        HardwareProfile GenerateAIOptimizedProfile();
        std::vector<FingerprintComponent> GetAIRecommendedComponents();

        // Session management
        SpoofingSession GetCurrentSession() const;
        bool RestoreOriginalFingerprint();
        bool BackupCurrentFingerprint();

        // Metrics and monitoring
        DetectionMetrics GetMetrics() const { return m_metrics; }
        void ResetMetrics();
        void ExportSpoofingLog(const std::string& filePath);

        // Configuration
        void SetSpoofingLevel(SpoofingLevel level);
        void EnableComponent(FingerprintComponent component, bool enable);
        void SetProfileRefreshInterval(std::chrono::minutes interval);

    private:
        // Core spoofing operations
        bool ApplyHardwareProfile(const HardwareProfile& profile);
        bool RevertHardwareProfile();
        HardwareProfile CaptureCurrentHardware();

        // Individual component spoofing
        bool SpoofSMBIOSTable();
        bool SpoofDiskProperties();
        bool SpoofGPUProperties();
        bool SpoofNetworkProperties();
        bool SpoofSystemProperties();

        // API hooking and interception
        bool HookGetVolumeInformation();
        bool HookGetDiskFreeSpace();
        bool HookGetSystemInfo();
        bool HookRegQueryValue();
        bool HookWMIQueries();

        // Profile generation algorithms
        std::string GenerateRealisticSerial(const std::string& prefix, size_t length);
        std::string GenerateMAC();
        std::string GenerateUUID();
        std::string GenerateProcessorId();

        // Validation and consistency
        bool ValidateProfile(const HardwareProfile& profile);
        bool EnsureProfileConsistency(HardwareProfile& profile);
        bool CheckProfileRealism(const HardwareProfile& profile);

        // Persistence mechanisms
        bool SaveSpoofedDataToDisk();
        bool LoadSpoofedDataFromDisk();
        bool UpdateSystemRegistry();
        bool CreatePersistentHooks();

        // Utility methods
        std::vector<std::string> GetAvailableDrives();
        std::vector<std::string> GetNetworkAdapters();
        std::string GetOriginalValue(const std::string& key);
        bool SetSystemValue(const std::string& key, const std::string& value);
    };

    /**
     * @brief SMBIOS Table Spoofer for motherboard and BIOS information
     */
    class SMBIOSSpoofer {
    public:
        struct SMBIOSEntry {
            uint8_t type;
            uint8_t length;
            uint16_t handle;
            std::vector<uint8_t> data;
            std::vector<std::string> strings;
        };

        struct SMBIOSTable {
            std::vector<SMBIOSEntry> entries;
            uint32_t tableLength;
            uint16_t structureCount;
            std::string version;
        };

    private:
        SMBIOSTable m_originalTable;
        SMBIOSTable m_spoofedTable;
        bool m_isActive;

    public:
        bool Initialize();
        bool BackupOriginalTable();
        bool ApplySpoofedTable(const SMBIOSTable& table);
        bool RestoreOriginalTable();
        SMBIOSTable GenerateRandomTable();
        bool ValidateTable(const SMBIOSTable& table);
    };

    /**
     * @brief Disk Serial Number Spoofer
     */
    class DiskSerialSpoofer {
    public:
        struct DiskInfo {
            std::string driveLetter;
            std::string volumeSerial;
            std::string volumeLabel;
            std::string fileSystem;
            uint64_t totalSize;
            uint64_t freeSpace;
        };

    private:
        std::vector<DiskInfo> m_originalDisks;
        std::vector<DiskInfo> m_spoofedDisks;
        std::unordered_map<std::string, HANDLE> m_hookHandles;

    public:
        bool Initialize();
        bool BackupOriginalSerials();
        bool ApplySpoofedSerials(const std::vector<std::string>& newSerials);
        bool RestoreOriginalSerials();
        std::vector<std::string> GenerateRandomSerials(size_t count);
        bool InstallVolumeHooks();
        bool RemoveVolumeHooks();
    };

    /**
     * @brief GPU Identity Masker for graphics card spoofing
     */
    class GPUIdentityMasker {
    public:
        struct GPUInfo {
            std::string deviceId;
            std::string vendorId;
            std::string deviceName;
            std::string driverVersion;
            uint64_t videoMemory;
            std::string subsystemId;
        };

    private:
        std::vector<GPUInfo> m_originalGPUs;
        std::vector<GPUInfo> m_spoofedGPUs;
        std::unordered_map<std::string, HANDLE> m_registryHooks;

    public:
        bool Initialize();
        bool BackupOriginalGPUInfo();
        bool ApplySpoofedGPUInfo(const std::vector<GPUInfo>& newGPUs);
        bool RestoreOriginalGPUInfo();
        std::vector<GPUInfo> GenerateRandomGPUs(size_t count);
        bool HookD3DQueries();
        bool HookOpenGLQueries();
        bool RemoveGraphicsHooks();
    };

    /**
     * @brief Network Adapter Spoofer for MAC address manipulation
     */
    class NetworkAdapterSpoofer {
    public:
        struct NetworkAdapter {
            std::string name;
            std::string macAddress;
            std::string description;
            bool isEnabled;
            std::string ipAddress;
        };

    private:
        std::vector<NetworkAdapter> m_originalAdapters;
        std::vector<NetworkAdapter> m_spoofedAdapters;

    public:
        bool Initialize();
        bool BackupOriginalMACs();
        bool ApplySpoofedMACs(const std::vector<std::string>& newMACs);
        bool RestoreOriginalMACs();
        std::vector<std::string> GenerateRandomMACs(size_t count);
        bool UpdateRegistryMACs();
        bool ResetNetworkStack();
    };

} // namespace Aether::Security