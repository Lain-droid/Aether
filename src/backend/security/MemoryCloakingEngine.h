#pragma once

#include <windows.h>
#include <winternl.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include <functional>
#include "SecurityTypes.h"

namespace Aether::Security {

    // Forward declarations
    class AIController;
    class VADManipulator;
    class GuardPageManager;
    class MemoryObfuscator;

    /**
     * @brief Advanced Memory Cloaking Engine for Hyperion Evasion
     * @details Provides sophisticated memory hiding techniques using VAD manipulation,
     *          Guard Pages, and custom memory management to remain invisible to
     *          NtQueryVirtualMemory and ReadProcessMemory scans
     */
    class MemoryCloakingEngine {
    public:
        enum class CloakingTechnique {
            VADManipulation,        // Modify Virtual Address Descriptor
            GuardPages,            // Use guard pages to hide memory
            MemoryMirroring,       // Create memory mirrors
            SectionHiding,         // Hide sections from enumeration
            HeapObfuscation,       // Obfuscate heap structures
            StackCloaking,         // Cloak stack regions
            ModuleHiding,          // Hide loaded modules
            HollowProcess          // Process hollowing techniques
        };

        enum class MemoryType {
            ExecutorCode,          // Main executor code
            ScriptData,            // Luau script data
            AIData,               // AI model and data
            CommunicationBuffer,   // IPC buffers
            ConfigurationData,     // Configuration and settings
            TemporaryData,        // Temporary working data
            DecoyData             // Fake data to mislead scans
        };

        struct CloakedRegion {
            LPVOID baseAddress;
            SIZE_T size;
            MemoryType type;
            CloakingTechnique technique;
            DWORD originalProtection;
            std::vector<LPVOID> guardPages;
            std::chrono::steady_clock::time_point creationTime;
            bool isActive;
            uint32_t accessCount;
        };

        struct VADEntry {
            LPVOID startVPN;
            LPVOID endVPN;
            DWORD protection;
            DWORD vadType;
            bool isHidden;
            LPVOID originalVAD;
        };

        struct MemoryMetrics {
            uint64_t totalCloakedMemory;
            uint32_t activeCloakedRegions;
            uint32_t memoryScansDetected;
            uint32_t successfulHides;
            uint32_t vadManipulations;
            uint32_t guardPageTriggers;
            std::chrono::milliseconds totalCloakingTime;
        };

    private:
        std::unique_ptr<AI::AIController> m_aiController;
        std::unique_ptr<VADManipulator> m_vadManipulator;
        std::unique_ptr<GuardPageManager> m_guardPageManager;
        std::unique_ptr<MemoryObfuscator> m_memoryObfuscator;

        std::vector<CloakedRegion> m_cloakedRegions;
        std::unordered_map<LPVOID, VADEntry> m_vadEntries;
        MemoryMetrics m_metrics;

        mutable std::mutex m_regionsMutex;
        mutable std::mutex m_vadMutex;

        // Hook information
        struct HookInfo {
            LPVOID originalFunction;
            LPVOID hookFunction;
            std::vector<BYTE> originalBytes;
            bool isActive;
        };

        std::unordered_map<std::string, HookInfo> m_installedHooks;

        // Configuration
        struct CloakingConfig {
            bool enableVADManipulation = true;
            bool enableGuardPages = true;
            bool enableMemoryMirroring = true;
            bool enableAIGuidedCloaking = true;
            uint32_t maxCloakedRegions = 1000;
            SIZE_T maxCloakedMemory = 1024 * 1024 * 1024; // 1GB
            std::chrono::milliseconds scanDetectionThreshold{100};
        } m_config;

    public:
        explicit MemoryCloakingEngine(std::shared_ptr<AI::AIController> aiController);
        ~MemoryCloakingEngine();

        // Initialization and lifecycle
        bool Initialize();
        void Shutdown();
        bool IsInitialized() const;

        // Memory cloaking operations
        bool CloakMemoryRegion(LPVOID baseAddress, SIZE_T size, MemoryType type,
                              CloakingTechnique technique = CloakingTechnique::VADManipulation);
        bool UncloakMemoryRegion(LPVOID baseAddress);
        bool UncloakAllRegions();

        // Specialized cloaking methods
        bool CloakExecutorCode(LPVOID codeBase, SIZE_T codeSize);
        bool CloakScriptData(LPVOID dataBase, SIZE_T dataSize);
        bool CloakAIComponents(LPVOID aiBase, SIZE_T aiSize);
        bool CloakCommunicationBuffers(LPVOID bufferBase, SIZE_T bufferSize);

        // VAD manipulation
        bool HideFromVADTree(LPVOID baseAddress, SIZE_T size);
        bool RestoreVADEntry(LPVOID baseAddress);
        bool ModifyVADProtection(LPVOID baseAddress, DWORD newProtection);

        // Guard page management
        bool InstallGuardPages(LPVOID baseAddress, SIZE_T size);
        bool RemoveGuardPages(LPVOID baseAddress);
        bool HandleGuardPageException(LPVOID faultAddress);

        // Memory obfuscation
        bool ObfuscateMemoryContent(LPVOID baseAddress, SIZE_T size);
        bool DeobfuscateMemoryContent(LPVOID baseAddress, SIZE_T size);
        bool RotateObfuscationKeys();

        // Detection and evasion
        bool DetectMemoryScanning();
        void ActivateAntiScanningMeasures();
        bool InstallMemoryHooks();
        bool RemoveMemoryHooks();

        // AI integration
        CloakingTechnique SelectOptimalTechnique(MemoryType type, SIZE_T size);
        void AdaptToScanningPattern();
        void UpdateAIModel();

        // Utility and management
        std::vector<CloakedRegion> GetCloakedRegions() const;
        CloakedRegion* FindCloakedRegion(LPVOID address);
        bool ValidateMemoryIntegrity();

        // Metrics and monitoring
        MemoryMetrics GetMetrics() const { return m_metrics; }
        void ResetMetrics();
        void ExportCloakingLog(const std::string& filePath);

        // Configuration
        void SetMaxCloakedRegions(uint32_t maxRegions) { m_config.maxCloakedRegions = maxRegions; }
        void EnableTechnique(CloakingTechnique technique, bool enable);
        void SetScanDetectionThreshold(std::chrono::milliseconds threshold);

    private:
        // Core cloaking implementations
        bool ApplyVADCloaking(CloakedRegion& region);
        bool ApplyGuardPageCloaking(CloakedRegion& region);
        bool ApplyMemoryMirrorCloaking(CloakedRegion& region);
        bool ApplySectionHiding(CloakedRegion& region);

        // Hook implementations
        bool HookNtQueryVirtualMemory();
        bool HookReadProcessMemory();
        bool HookVirtualQueryEx();
        bool HookNtMapViewOfSection();
        bool HookNtUnmapViewOfSection();

        // Hook handlers
        static NTSTATUS NTAPI HookedNtQueryVirtualMemory(
            HANDLE ProcessHandle, PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass,
            PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength);
        
        static BOOL WINAPI HookedReadProcessMemory(
            HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer,
            SIZE_T nSize, SIZE_T* lpNumberOfBytesRead);

        // VAD manipulation helpers
        bool LocateVADEntry(LPVOID address, PVOID* vadEntry);
        bool ModifyVADEntry(PVOID vadEntry, const VADEntry& newEntry);
        bool UnlinkVADEntry(PVOID vadEntry);
        bool RelinkVADEntry(PVOID vadEntry);

        // Memory scanning detection
        bool DetectSequentialMemoryAccess();
        bool DetectKnownScanningPatterns();
        bool AnalyzeMemoryAccessPattern();

        // Anti-scanning countermeasures
        void ScrambleMemoryLayout();
        void InsertDecoyMemoryRegions();
        void ActivateMemoryMisdirection();

        // Utility methods
        LPVOID AllocateHiddenMemory(SIZE_T size, DWORD protection);
        bool FreeHiddenMemory(LPVOID address);
        bool IsAddressInCloakedRegion(LPVOID address);
        DWORD CalculateRegionHash(LPVOID baseAddress, SIZE_T size);
    };

    /**
     * @brief VAD (Virtual Address Descriptor) Manipulator for Windows kernel structures
     */
    class VADManipulator {
    public:
        enum class VADType {
            VadNone = 0,
            VadDevicePhysicalMemory = 1,
            VadImageMap = 2,
            VadAwe = 3,
            VadWriteWatch = 4,
            VadLargePages = 5,
            VadRotatePhysical = 6,
            VadLargePageSection = 7
        };

        struct VADNode {
            LPVOID vadPtr;
            LPVOID startVPN;
            LPVOID endVPN;
            VADType vadType;
            DWORD protection;
            bool isHidden;
        };

    private:
        std::vector<VADNode> m_manipulatedVADs;
        std::unordered_map<LPVOID, LPVOID> m_originalVADs;

    public:
        bool Initialize();
        bool HideVADNode(LPVOID baseAddress, SIZE_T size);
        bool RestoreVADNode(LPVOID baseAddress);
        bool ModifyVADProtection(LPVOID baseAddress, DWORD newProtection);
        std::vector<VADNode> GetManipulatedVADs() const;
    };

    /**
     * @brief Guard Page Manager for memory access control
     */
    class GuardPageManager {
    public:
        struct GuardPageInfo {
            LPVOID pageAddress;
            SIZE_T pageSize;
            DWORD originalProtection;
            bool isActive;
            uint32_t triggerCount;
            std::chrono::steady_clock::time_point lastTrigger;
        };

    private:
        std::vector<GuardPageInfo> m_guardPages;
        PVOID m_exceptionHandler;

    public:
        bool Initialize();
        bool InstallGuardPage(LPVOID address, SIZE_T size);
        bool RemoveGuardPage(LPVOID address);
        bool HandleException(LPEXCEPTION_POINTERS exceptionInfo);
        std::vector<GuardPageInfo> GetActiveGuardPages() const;
    };

    /**
     * @brief Memory Obfuscator for content protection
     */
    class MemoryObfuscator {
    public:
        enum class ObfuscationType {
            XORCipher,
            AESEncryption,
            PolymorphicXOR,
            BitwiseScrambling,
            HuffmanCompression
        };

        struct ObfuscationContext {
            ObfuscationType type;
            std::vector<BYTE> key;
            std::vector<BYTE> iv;
            SIZE_T dataSize;
            bool isObfuscated;
        };

    private:
        std::unordered_map<LPVOID, ObfuscationContext> m_obfuscationMap;
        std::vector<BYTE> m_masterKey;

    public:
        bool Initialize();
        bool ObfuscateRegion(LPVOID baseAddress, SIZE_T size, ObfuscationType type);
        bool DeobfuscateRegion(LPVOID baseAddress);
        bool RotateKeys();
        ObfuscationContext GetObfuscationInfo(LPVOID address);
    };

} // namespace Aether::Security