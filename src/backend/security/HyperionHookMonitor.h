#pragma once

#include <windows.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include "SecurityTypes.h"

namespace Aether::Security {

    // Forward declarations
    class AIController;
    class InlinePatcher;
    class APIDetourManager;
    class SelfHealingSystem;

    /**
     * @brief Advanced Hook Monitor specifically designed to detect and counter Hyperion anti-cheat
     * @details This system monitors for Hyperion hooks, patches, and scanning attempts in real-time
     */
    class HyperionHookMonitor {
    public:
        enum class ThreatLevel {
            None = 0,
            Low = 1,
            Medium = 2,
            High = 3,
            Critical = 4
        };

        enum class HyperionComponent {
            AntiTamper,
            MemoryScanner,
            ProcessMonitor,
            NetworkAnalyzer,
            BehaviorAnalyzer,
            SignatureScanner
        };

        struct HookDetection {
            DWORD processId;
            LPVOID hookAddress;
            LPVOID originalFunction;
            HyperionComponent component;
            ThreatLevel threatLevel;
            std::chrono::steady_clock::time_point detectionTime;
            std::vector<BYTE> originalBytes;
            std::vector<BYTE> patchedBytes;
        };

        struct AntiTamperThread {
            HANDLE threadHandle;
            DWORD threadId;
            LPVOID startAddress;
            HyperionComponent component;
            bool isActive;
            std::chrono::steady_clock::time_point lastActivity;
        };

    private:
        std::unique_ptr<AIController> m_aiController;
        std::unique_ptr<InlinePatcher> m_inlinePatcher;
        std::unique_ptr<APIDetourManager> m_apiDetourManager;
        std::unique_ptr<SelfHealingSystem> m_selfHealingSystem;

        std::atomic<bool> m_isMonitoring;
        std::thread m_monitorThread;
        std::thread m_hyperionDetectionThread;
        std::thread m_selfHealingThread;

        mutable std::mutex m_detectionsMutex;
        mutable std::mutex m_threadsMutex;
        
        std::vector<HookDetection> m_detectedHooks;
        std::vector<AntiTamperThread> m_hyperionThreads;
        std::unordered_map<LPVOID, LPVOID> m_originalFunctions;

        // AI-enhanced detection patterns
        std::vector<std::vector<BYTE>> m_hyperionSignatures;
        std::unordered_map<HyperionComponent, std::vector<std::string>> m_knownApiTargets;

        // Performance metrics for AI learning
        struct DetectionMetrics {
            uint32_t totalDetections;
            uint32_t falsePositives;
            uint32_t successfulBypasses;
            uint32_t failedBypasses;
            std::chrono::milliseconds averageDetectionTime;
        } m_metrics;

    public:
        explicit HyperionHookMonitor(std::shared_ptr<AIController> aiController);
        ~HyperionHookMonitor();

        // Core monitoring functionality
        bool Initialize();
        void StartMonitoring();
        void StopMonitoring();
        bool IsMonitoring() const { return m_isMonitoring.load(); }

        // Hyperion-specific detection
        bool DetectHyperionPresence();
        std::vector<AntiTamperThread> ScanForAntiTamperThreads();
        bool AnalyzeThreadBehavior(const AntiTamperThread& thread);
        
        // Hook detection and analysis
        std::vector<HookDetection> ScanForHooks();
        bool AnalyzeHookPattern(const HookDetection& hook);
        ThreatLevel AssessThreatLevel(const HookDetection& hook);

        // Dynamic bypass mechanisms
        bool BypassAntiTamperThread(const AntiTamperThread& thread);
        bool ApplyInlinePatch(LPVOID targetAddress, const std::vector<BYTE>& patchBytes);
        bool InstallAPIDetour(const std::string& apiName, LPVOID newFunction);
        bool RemoveAPIDetour(const std::string& apiName);

        // Self-healing capabilities
        bool TriggerSelfHealing();
        bool RollbackChanges();
        bool RestoreOriginalState();

        // AI integration
        void FeedDetectionDataToAI(const HookDetection& detection);
        void UpdateAIModel();
        std::vector<BYTE> GenerateAICountermeasure(const HookDetection& hook);

        // Metrics and reporting
        DetectionMetrics GetMetrics() const { return m_metrics; }
        std::vector<HookDetection> GetRecentDetections(std::chrono::minutes timeWindow) const;
        void ExportDetectionLog(const std::string& filePath) const;

    private:
        // Internal monitoring loops
        void MonitoringLoop();
        void HyperionDetectionLoop();
        void SelfHealingLoop();

        // Low-level detection methods
        bool ScanProcessMemory();
        bool CheckAPIIntegrity();
        bool AnalyzeExecutionFlow();
        bool DetectDebuggerAttachment();

        // Pattern recognition
        bool MatchHyperionSignature(const std::vector<BYTE>& data);
        HyperionComponent IdentifyHyperionComponent(const std::vector<BYTE>& signature);
        
        // Advanced evasion techniques
        bool UsePolymorphicEvasion();
        bool ApplyAntiAnalysis();
        bool ImplementDecoyRoutines();

        // Utility methods
        std::vector<BYTE> ReadMemoryBytes(LPVOID address, size_t size);
        bool WriteMemoryBytes(LPVOID address, const std::vector<BYTE>& bytes);
        DWORD GetProcessIdByName(const std::wstring& processName);
        std::vector<HANDLE> GetProcessThreads(DWORD processId);
    };

    /**
     * @brief Inline Patcher for dynamic anti-tamper thread bypassing
     */
    class InlinePatcher {
    public:
        struct PatchInfo {
            LPVOID targetAddress;
            std::vector<BYTE> originalBytes;
            std::vector<BYTE> patchBytes;
            bool isActive;
            std::chrono::steady_clock::time_point appliedTime;
        };

    private:
        std::unordered_map<LPVOID, PatchInfo> m_activePatchs;
        mutable std::mutex m_patchMutex;

    public:
        bool ApplyPatch(LPVOID targetAddress, const std::vector<BYTE>& patchBytes);
        bool RemovePatch(LPVOID targetAddress);
        bool RestoreAllPatches();
        std::vector<PatchInfo> GetActivePatches() const;
    };

    /**
     * @brief API Detour Manager for function redirection
     */
    class APIDetourManager {
    public:
        struct DetourInfo {
            std::string apiName;
            LPVOID originalFunction;
            LPVOID detourFunction;
            bool isActive;
            std::chrono::steady_clock::time_point installedTime;
        };

    private:
        std::unordered_map<std::string, DetourInfo> m_activeDetours;
        mutable std::mutex m_detourMutex;

    public:
        bool InstallDetour(const std::string& apiName, LPVOID detourFunction);
        bool RemoveDetour(const std::string& apiName);
        bool RemoveAllDetours();
        std::vector<DetourInfo> GetActiveDetours() const;
    };

    /**
     * @brief Self-Healing System for automatic rollback
     */
    class SelfHealingSystem {
    public:
        enum class HealingAction {
            RollbackPatch,
            RemoveDetour,
            RestoreMemory,
            TerminateThread,
            RestartComponent
        };

        struct HealingRule {
            ThreatLevel triggerLevel;
            HealingAction action;
            std::chrono::milliseconds delay;
            bool isAutomatic;
        };

    private:
        std::vector<HealingRule> m_healingRules;
        std::atomic<bool> m_isActive;
        mutable std::mutex m_rulesMutex;

    public:
        void AddHealingRule(const HealingRule& rule);
        void RemoveHealingRule(ThreatLevel triggerLevel, HealingAction action);
        bool TriggerHealing(ThreatLevel currentThreat);
        void SetAutomatic(bool automatic) { m_isActive.store(automatic); }
    };
}