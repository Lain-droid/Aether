#pragma once

#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <queue>
#include "AIController.h"
#include "SecurityAIOrchestrator.h"

namespace Aether::AI {

    /**
     * @brief Unified AI Synchronization Framework
     * @details Master synchronization system that ensures ALL components are perfectly
     *          synchronized with the AI brain. NO COMPONENT can operate without AI coordination.
     */
    class UnifiedAISynchronizer {
    public:
        enum class ComponentType {
            CORE_SYSTEM,
            MEMORY_MANAGER,
            NETWORK_LAYER,
            IPC_SYSTEM,
            VM_COMPILER,
            VM_RUNTIME,
            HOOKS_MANAGER,
            SECURITY_LAYER,
            OBFUSCATION_ENGINE,
            BEHAVIORAL_AI,
            PATTERN_DETECTOR,
            THREAT_ANALYZER
        };

        enum class SyncPriority {
            CRITICAL = 0,    // Immediate sync required
            HIGH = 1,        // Sync within 10ms
            MEDIUM = 2,      // Sync within 100ms
            LOW = 3          // Sync within 1000ms
        };

        struct ComponentRegistration {
            ComponentType type;
            std::string name;
            void* componentPtr;
            SyncPriority priority;
            std::function<void()> syncCallback;
            std::function<void(const ThreatAssessment&)> threatCallback;
            std::function<void(Backend::AIEventType)> eventCallback;
            std::chrono::steady_clock::time_point lastSync;
            bool isActive;
            uint64_t syncCount;
        };

        struct SyncMetrics {
            uint64_t totalSyncs = 0;
            uint64_t missedSyncs = 0;
            uint64_t emergencySyncs = 0;
            std::chrono::microseconds avgSyncTime{0};
            std::chrono::microseconds maxSyncTime{0};
            double syncEfficiency = 1.0;
        };

        struct GlobalAIState {
            Backend::RiskLevel currentRiskLevel;
            SecurityAIOrchestrator::ThreatLevel threatLevel;
            SecurityAIOrchestrator::AIStrategy activeStrategy;
            std::map<ComponentType, double> componentHealthScores;
            std::chrono::steady_clock::time_point lastGlobalUpdate;
            bool emergencyMode = false;
            bool stealthMode = false;
        };

    private:
        static std::unique_ptr<UnifiedAISynchronizer> s_instance;
        static std::mutex s_instanceMutex;

        std::shared_ptr<Backend::AIController> m_masterAI;
        std::unique_ptr<SecurityAIOrchestrator> m_orchestrator;
        
        std::map<ComponentType, ComponentRegistration> m_registeredComponents;
        std::queue<std::pair<ComponentType, SyncPriority>> m_syncQueue;
        
        GlobalAIState m_globalState;
        SyncMetrics m_metrics;
        
        mutable std::mutex m_componentsMutex;
        mutable std::mutex m_queueMutex;
        mutable std::mutex m_stateMutex;
        
        std::vector<std::thread> m_syncThreads;
        std::atomic<bool> m_isRunning{false};
        std::atomic<bool> m_emergencyMode{false};
        
        // High-priority sync thread for critical components
        std::thread m_criticalSyncThread;
        std::thread m_monitoringThread;
        std::thread m_emergencyResponseThread;

    public:
        static UnifiedAISynchronizer& GetInstance();
        static void Initialize(std::shared_ptr<Backend::AIController> masterAI);
        static void Shutdown();

        // Component registration and management
        bool RegisterComponent(ComponentType type, const std::string& name, void* componentPtr,
                             SyncPriority priority, std::function<void()> syncCallback);
        
        bool RegisterComponentWithCallbacks(ComponentType type, const std::string& name, void* componentPtr,
                                           SyncPriority priority,
                                           std::function<void()> syncCallback,
                                           std::function<void(const ThreatAssessment&)> threatCallback,
                                           std::function<void(Backend::AIEventType)> eventCallback);
        
        void UnregisterComponent(ComponentType type);
        bool IsComponentRegistered(ComponentType type) const;
        void SetComponentActive(ComponentType type, bool active);

        // Synchronization control
        void StartSynchronization();
        void StopSynchronization();
        void ForceSyncAll();
        void ForceSyncComponent(ComponentType type);
        void RequestSync(ComponentType type, SyncPriority priority);

        // Emergency and threat response
        void TriggerEmergencySync();
        void ActivateEmergencyMode();
        void DeactivateEmergencyMode();
        void ProcessThreatUpdate(const ThreatAssessment& threat);
        void BroadcastAIEvent(Backend::AIEventType event);

        // Global state management
        GlobalAIState GetGlobalState() const;
        void UpdateGlobalRiskLevel(Backend::RiskLevel newLevel);
        void UpdateThreatLevel(SecurityAIOrchestrator::ThreatLevel newLevel);
        void UpdateAIStrategy(SecurityAIOrchestrator::AIStrategy newStrategy);
        void ActivateStealthMode(bool activate);

        // Monitoring and metrics
        SyncMetrics GetSyncMetrics() const;
        void ResetMetrics();
        std::map<ComponentType, double> GetComponentHealthScores() const;
        double CalculateOverallSyncHealth() const;
        bool IsSyncSystemHealthy() const;

        // Advanced synchronization features
        void SetSyncInterval(ComponentType type, std::chrono::milliseconds interval);
        void SetPriorityThresholds(const std::map<SyncPriority, std::chrono::milliseconds>& thresholds);
        void EnableAdaptiveSync(bool enable);
        void OptimizeSyncScheduling();

        // Component health monitoring
        void UpdateComponentHealth(ComponentType type, double healthScore);
        std::vector<ComponentType> GetUnhealthyComponents() const;
        void QuarantineComponent(ComponentType type);
        void RestoreComponent(ComponentType type);

    private:
        UnifiedAISynchronizer() = default;
        ~UnifiedAISynchronizer();

        // Core synchronization methods
        void CriticalSyncLoop();
        void StandardSyncLoop();
        void MonitoringLoop();
        void EmergencyResponseLoop();

        // Synchronization helpers
        void ExecuteSync(ComponentType type);
        void ProcessSyncQueue();
        void HandleMissedSync(ComponentType type);
        void UpdateSyncMetrics(ComponentType type, std::chrono::microseconds syncTime);

        // Emergency response
        void HandleComponentFailure(ComponentType type);
        void InitiateSystemRecovery();
        void PerformEmergencyShutdown();

        // Health monitoring
        void MonitorComponentHealth();
        void AnalyzeSyncPatterns();
        void DetectSyncAnomalies();
        void OptimizeComponentScheduling();

        // Utility methods
        std::chrono::milliseconds GetSyncInterval(SyncPriority priority) const;
        bool ShouldSyncComponent(const ComponentRegistration& component) const;
        void LogSyncEvent(ComponentType type, const std::string& event);
    };

    /**
     * @brief Component Registration Helper Template
     * @details Simplifies registration of components with automatic type detection
     */
    template<typename T>
    class ComponentSyncWrapper {
    private:
        T* m_component;
        ComponentType m_type;
        bool m_registered = false;

    public:
        ComponentSyncWrapper(T* component, ComponentType type) 
            : m_component(component), m_type(type) {}

        ~ComponentSyncWrapper() {
            if (m_registered) {
                UnregisterFromSync();
            }
        }

        bool RegisterWithSync(const std::string& name, SyncPriority priority,
                            std::function<void()> syncCallback) {
            auto& synchronizer = UnifiedAISynchronizer::GetInstance();
            m_registered = synchronizer.RegisterComponent(m_type, name, m_component, priority, syncCallback);
            return m_registered;
        }

        bool RegisterWithFullCallbacks(const std::string& name, SyncPriority priority,
                                     std::function<void()> syncCallback,
                                     std::function<void(const ThreatAssessment&)> threatCallback,
                                     std::function<void(Backend::AIEventType)> eventCallback) {
            auto& synchronizer = UnifiedAISynchronizer::GetInstance();
            m_registered = synchronizer.RegisterComponentWithCallbacks(m_type, name, m_component, 
                                                                      priority, syncCallback, 
                                                                      threatCallback, eventCallback);
            return m_registered;
        }

        void UnregisterFromSync() {
            if (m_registered) {
                auto& synchronizer = UnifiedAISynchronizer::GetInstance();
                synchronizer.UnregisterComponent(m_type);
                m_registered = false;
            }
        }

        void RequestSync(SyncPriority priority = SyncPriority::MEDIUM) {
            if (m_registered) {
                auto& synchronizer = UnifiedAISynchronizer::GetInstance();
                synchronizer.RequestSync(m_type, priority);
            }
        }

        T* GetComponent() { return m_component; }
        ComponentType GetType() const { return m_type; }
        bool IsRegistered() const { return m_registered; }
    };

    // Convenience macros for registration
    #define REGISTER_COMPONENT_SYNC(component, type, name, priority, callback) \
        UnifiedAISynchronizer::GetInstance().RegisterComponent(type, name, component, priority, callback)

    #define REGISTER_COMPONENT_FULL_SYNC(component, type, name, priority, sync_cb, threat_cb, event_cb) \
        UnifiedAISynchronizer::GetInstance().RegisterComponentWithCallbacks(type, name, component, priority, sync_cb, threat_cb, event_cb)

    #define REQUEST_SYNC(type, priority) \
        UnifiedAISynchronizer::GetInstance().RequestSync(type, priority)

    #define FORCE_EMERGENCY_SYNC() \
        UnifiedAISynchronizer::GetInstance().TriggerEmergencySync()

} // namespace Aether::AI