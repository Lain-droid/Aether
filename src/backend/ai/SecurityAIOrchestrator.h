#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include "AIController.h"
#include "../security/HyperionHookMonitor.h"
#include "../virtualization/PolymorphicVirtualMachine.h"
#include "../security/HardwareFingerprintSpoofer.h"
#include "../security/MemoryCloakingEngine.h"
#include "BehaviorRandomizer.h"
#include "../loader/SelfDeletingLoader.h"
#include "../ipc/EncryptedSharedMemoryIPC.h"
#include "../mutation/RuntimeBinaryMutator.h"
#include "../security/DecoyLayerSystem.h"

namespace Aether::AI {

    /**
     * @brief Security AI Orchestrator - Master AI Controller for All Security Features
     * @details Provides centralized AI coordination and control for all advanced security
     *          components, enabling adaptive, intelligent, and coordinated defense strategies
     */
    class SecurityAIOrchestrator {
    public:
        enum class ThreatLevel {
            None = 0,
            Low = 1,
            Medium = 2,
            High = 3,
            Critical = 4,
            Emergency = 5
        };

        enum class SecurityComponent {
            HyperionMonitor,
            VirtualMachine,
            FingerprintSpoofer,
            MemoryCloaking,
            BehaviorRandomizer,
            SelfDeletingLoader,
            EncryptedIPC,
            BinaryMutator,
            DecoyLayer
        };

        enum class AIStrategy {
            Defensive,         // Focus on evasion and hiding
            Aggressive,        // Active countermeasures
            Adaptive,          // Dynamic strategy adjustment
            Stealth,           // Maximum concealment
            Deceptive,         // Focus on misdirection
            Balanced          // Balanced approach
        };

        struct ThreatAssessment {
            ThreatLevel level;
            std::vector<SecurityComponent> threatenedComponents;
            std::vector<std::string> detectedMethods;
            std::chrono::steady_clock::time_point detectionTime;
            double confidence;
            std::string description;
        };

        struct SecurityMetrics {
            uint64_t totalThreats;
            uint64_t neutralizedThreats;
            uint64_t activeCountermeasures;
            uint64_t adaptiveAdjustments;
            double overallSecurity;
            double adaptationSpeed;
            std::chrono::milliseconds responseTime;
            std::unordered_map<SecurityComponent, double> componentEffectiveness;
        };

        struct AIConfiguration {
            AIStrategy strategy;
            bool enableRealTimeAdaptation;
            bool enablePredictiveDefense;
            bool enableCoordinatedResponse;
            bool enableLearning;
            std::chrono::milliseconds responseThreshold;
            double threatSensitivity;
            uint32_t maxConcurrentActions;
        };

    private:
        // Core AI components
        std::unique_ptr<AIController> m_aiController;
        
        // Security components
        std::unique_ptr<Security::HyperionHookMonitor> m_hyperionMonitor;
        std::unique_ptr<Virtualization::PolymorphicVirtualMachine> m_virtualMachine;
        std::unique_ptr<Security::HardwareFingerprintSpoofer> m_fingerprintSpoofer;
        std::unique_ptr<Security::MemoryCloakingEngine> m_memoryCloaking;
        std::unique_ptr<BehaviorRandomizer> m_behaviorRandomizer;
        std::unique_ptr<Loader::SelfDeletingLoader> m_selfDeletingLoader;
        std::unique_ptr<IPC::EncryptedSharedMemoryIPC> m_encryptedIPC;
        std::unique_ptr<Mutation::RuntimeBinaryMutator> m_binaryMutator;
        std::unique_ptr<Security::DecoyLayerSystem> m_decoyLayer;

        // State management
        AIConfiguration m_config;
        ThreatAssessment m_currentThreat;
        SecurityMetrics m_metrics;
        
        std::atomic<bool> m_isActive;
        std::atomic<ThreatLevel> m_currentThreatLevel;
        
        mutable std::mutex m_threatMutex;
        mutable std::mutex m_metricsMutex;

        // AI coordination threads
        std::thread m_orchestrationThread;
        std::thread m_threatAnalysisThread;
        std::thread m_adaptationThread;
        std::thread m_coordinationThread;

        // Event system
        std::vector<std::function<void(const ThreatAssessment&)>> m_threatHandlers;
        std::vector<std::function<void(SecurityComponent, double)>> m_effectivenessHandlers;

    public:
        explicit SecurityAIOrchestrator();
        ~SecurityAIOrchestrator();

        // Initialization and lifecycle
        bool Initialize();
        void Shutdown();
        bool IsActive() const { return m_isActive.load(); }

        // Component registration and management
        bool RegisterComponent(SecurityComponent component, void* componentPtr);
        bool UnregisterComponent(SecurityComponent component);
        bool IsComponentActive(SecurityComponent component);

        // Threat assessment and response
        ThreatAssessment AssessThreat();
        bool RespondToThreat(const ThreatAssessment& threat);
        bool EscalateThreatLevel(ThreatLevel newLevel);
        bool ActivateEmergencyProtocol();

        // AI strategy and coordination
        bool SetAIStrategy(AIStrategy strategy);
        bool AdaptStrategy();
        bool CoordinateComponentActions();
        bool OptimizeSecurityConfiguration();

        // Predictive capabilities
        bool PredictThreat(std::chrono::milliseconds timeWindow);
        std::vector<ThreatAssessment> GetThreatPredictions();
        bool PreemptiveCountermeasures();

        // Learning and adaptation
        bool UpdateThreatModel(const ThreatAssessment& threat, bool successful);
        bool LearnFromComponent(SecurityComponent component);
        bool AdaptToNewThreats();
        bool ExportLearningData(const std::string& filePath);

        // Component control
        bool ActivateComponent(SecurityComponent component);
        bool DeactivateComponent(SecurityComponent component);
        bool ReconfigureComponent(SecurityComponent component, const std::string& parameters);
        bool TriggerComponentAction(SecurityComponent component, const std::string& action);

        // Coordinated responses
        bool ActivateStealthMode();
        bool ActivateAggressiveMode();
        bool ActivateDeceptionMode();
        bool ActivateEmergencyEvasion();

        // Metrics and monitoring
        SecurityMetrics GetMetrics() const;
        double CalculateOverallSecurity();
        std::vector<SecurityComponent> GetWeakestComponents();
        void ExportSecurityReport(const std::string& filePath);

        // Event system
        void RegisterThreatHandler(std::function<void(const ThreatAssessment&)> handler);
        void RegisterEffectivenessHandler(std::function<void(SecurityComponent, double)> handler);

        // Configuration
        void SetConfiguration(const AIConfiguration& config) { m_config = config; }
        AIConfiguration GetConfiguration() const { return m_config; }

    private:
        // Core orchestration methods
        void OrchestrationLoop();
        void ThreatAnalysisLoop();
        void AdaptationLoop();
        void CoordinationLoop();

        // Threat assessment implementations
        ThreatAssessment AnalyzeCurrentThreats();
        ThreatLevel CalculateThreatLevel();
        std::vector<SecurityComponent> IdentifyThreatenedComponents();
        double CalculateConfidence(const ThreatAssessment& threat);

        // Response implementations
        bool ExecuteDefensiveStrategy();
        bool ExecuteAggressiveStrategy();
        bool ExecuteAdaptiveStrategy();
        bool ExecuteStealthStrategy();
        bool ExecuteDeceptiveStrategy();

        // Component coordination
        bool SynchronizeComponents();
        bool DistributeAIDecisions();
        bool CoordinateComponentStates();
        bool OptimizeComponentInteractions();

        // Learning implementations
        bool UpdateModelWeights();
        bool AnalyzeComponentEffectiveness();
        bool LearnFromSuccessfulDefenses();
        bool LearnFromFailedDefenses();

        // Utility methods
        bool IsComponentRegistered(SecurityComponent component);
        void* GetComponentPointer(SecurityComponent component);
        std::string ComponentToString(SecurityComponent component);
        void EmitThreatEvent(const ThreatAssessment& threat);
        void EmitEffectivenessEvent(SecurityComponent component, double effectiveness);
    };

    /**
     * @brief AI Decision Engine for intelligent security decisions
     */
    class AIDecisionEngine {
    public:
        enum class DecisionType {
            ComponentActivation,
            StrategyChange,
            ThreatResponse,
            ResourceAllocation,
            ConfigurationUpdate
        };

        struct Decision {
            DecisionType type;
            SecurityAIOrchestrator::SecurityComponent targetComponent;
            std::string action;
            std::unordered_map<std::string, std::string> parameters;
            double confidence;
            std::chrono::steady_clock::time_point timestamp;
        };

    private:
        std::vector<Decision> m_decisionHistory;
        std::shared_ptr<AIController> m_aiController;

    public:
        explicit AIDecisionEngine(std::shared_ptr<AIController> aiController);
        Decision MakeDecision(const SecurityAIOrchestrator::ThreatAssessment& threat);
        std::vector<Decision> GetDecisionHistory();
        double AnalyzeDecisionEffectiveness();
    };

    /**
     * @brief Threat Prediction Engine for predictive defense
     */
    class ThreatPredictionEngine {
    public:
        struct PredictionModel {
            std::vector<double> weights;
            std::vector<std::string> features;
            double accuracy;
            std::chrono::steady_clock::time_point lastUpdate;
        };

        struct ThreatPrediction {
            SecurityAIOrchestrator::ThreatLevel predictedLevel;
            std::chrono::milliseconds timeToThreat;
            double probability;
            std::vector<std::string> indicators;
        };

    private:
        PredictionModel m_model;
        std::shared_ptr<AIController> m_aiController;

    public:
        explicit ThreatPredictionEngine(std::shared_ptr<AIController> aiController);
        bool Initialize();
        ThreatPrediction PredictThreat(std::chrono::milliseconds timeWindow);
        bool UpdateModel(const std::vector<SecurityAIOrchestrator::ThreatAssessment>& threats);
        double GetModelAccuracy();
    };

    /**
     * @brief Component Optimization Engine for performance tuning
     */
    class ComponentOptimizationEngine {
    public:
        struct OptimizationResult {
            SecurityAIOrchestrator::SecurityComponent component;
            std::unordered_map<std::string, std::string> optimizedParameters;
            double expectedImprovement;
            double confidence;
        };

    private:
        std::shared_ptr<AIController> m_aiController;

    public:
        explicit ComponentOptimizationEngine(std::shared_ptr<AIController> aiController);
        OptimizationResult OptimizeComponent(SecurityAIOrchestrator::SecurityComponent component);
        std::vector<OptimizationResult> OptimizeAllComponents();
        bool ApplyOptimization(const OptimizationResult& optimization);
    };

} // namespace Aether::AI