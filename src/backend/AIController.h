#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <utility>
#ifdef _WIN32
#include <Windows.h>
#ifdef ReportEvent
#undef ReportEvent
#endif
#endif

namespace AetherVisor {
    namespace Backend {

        // Defines the perceived risk level based on monitored events.
        enum class RiskLevel {
            NONE,       // No suspicious activity detected.
            LOW,        // Minor, infrequent events.
            MEDIUM,     // Potentially risky patterns observed.
            HIGH,       // Active detection or high-profile API usage.
            CRITICAL    // Anti-cheat is likely alerted. All operations should be minimal.
        };

        // An identifier for the type of event being reported to the AI system.
        enum class FeedbackType {
            NONE,
            KICKED_FROM_GAME,
            HIGH_LATENCY_DETECTED,
            FUNCTION_CALL_FAILED,
            MEMORY_SCAN_DETECTED,
            BEHAVIORAL_ANOMALY_DETECTED,
            NETWORK_PATTERN_FLAGGED,
            ANTI_CHEAT_SIGNATURE_MATCH
        };

        enum class AIEventType {
            // Execution Events
            INJECTION_ATTEMPT,
            PAYLOAD_EXECUTED,
            HOOK_CALLED,
            SCRIPT_EXECUTION,
            FUNCTION_OVERRIDE,

            // Memory Events
            MEMORY_PATCH_APPLIED,
            MEMORY_READ,
            MEMORY_WRITE,
            MEMORY_SCAN_EVASION,
            PATTERN_OBFUSCATION,

            // Network Events
            NETWORK_PACKET_SENT,
            NETWORK_PACKET_RECEIVED,
            NETWORK_TIMING_ANOMALY,
            BANDWIDTH_SPIKE,

            // Potential Detection Events
            SUSPICIOUS_API_CALL,
            SERVER_THROTTLING_DETECTED,
            ANTI_CHEAT_PROBE,
            SIGNATURE_SCAN_ATTEMPT,
            BEHAVIORAL_ANALYSIS_DETECTED,
            
            // Advanced AI Events
            NEURAL_PREDICTION,
            PATTERN_LEARNING,
            ADAPTIVE_BEHAVIOR_CHANGE
        };

        /**
         * @class AIController
         * @brief Manages risk assessment and adaptive behavior.
         *
         * This class is the core of the "Advanced AI Layer". It collects data
         * about the application's actions and external events to calculate a
         * risk score. Other modules will query this controller to decide
         * whether to use high-risk or low-risk techniques.
         */
        class AIController {
        public:
            // Gets the singleton instance of the AIController.
            static AIController& GetInstance();

            /**
             * @brief Reports an event to the AI system for analysis.
             * @param eventType The type of event that occurred.
             */
            void ReportEvent(AIEventType eventType);

            /**
             * @brief Gets the current calculated risk level.
             * @return The current RiskLevel enum.
             */
            RiskLevel GetCurrentRiskLevel() const;

            /**
             * @brief Determines if an action should be performed based on risk.
             * @param requiredLevel The maximum risk level at which this action is considered safe.
             * @return True if the current risk is less than or equal to the required level.
             */
            bool ShouldPerformAction(RiskLevel requiredLevel) const;

            /**
             * @brief Reports negative feedback to the AI to adjust its learning.
             * @param type The type of negative feedback received.
             */
            void ReportNegativeFeedback(FeedbackType type);

            /**
             * @brief Advanced neural pattern prediction for future events.
             * @param timeHorizon How far into the future to predict (in seconds).
             * @return Predicted risk level for the specified time horizon.
             */
            RiskLevel PredictFutureRisk(double timeHorizon);

            /**
             * @brief Adaptive learning from behavioral patterns.
             * @param behaviorData Vector of recent behavioral data points.
             */
            void LearnFromBehavioralPatterns(const std::vector<double>& behaviorData);

            /**
             * @brief Generates optimal timing for actions to minimize detection.
             * @param actionType The type of action to be performed.
             * @return Optimal delay in milliseconds before performing the action.
             */
            double GetOptimalActionTiming(AIEventType actionType);

            /**
             * @brief Advanced polymorphic decision making for evasion strategies.
             * @return Recommended evasion strategy identifier.
             */
            int GetRecommendedEvasionStrategy();

            /**
             * @brief Neural network-based detection of anti-cheat behavior patterns.
             * @param systemEvents Recent system events for pattern analysis.
             * @return Confidence level that anti-cheat is actively scanning (0.0-1.0).
             */
            double DetectAntiCheatActivity(const std::vector<AIEventType>& systemEvents);

        private:
            // Constructor initializes the risk weights.
            AIController();
            ~AIController() = default;
            AIController(const AIController&) = delete;
            AIController& operator=(const AIController&) = delete;

            // The current risk score. A higher value means higher risk.
            double m_riskScore = 0.0;
            RiskLevel m_currentRiskLevel = RiskLevel::NONE;

            // Data structures for learning
            std::map<AIEventType, double> m_riskWeights;
            std::vector<AIEventType> m_recentEvents;
            static const size_t MAX_HISTORY_SIZE = 50;
            static const size_t PATTERN_ANALYSIS_WINDOW = 100;

            // Advanced AI learning structures
            std::vector<double> m_behavioralHistory;
            std::map<AIEventType, std::vector<double>> m_timingPatterns;
            std::vector<std::pair<std::vector<AIEventType>, double>> m_patternRiskMappings;
            
            // Neural network weights for pattern recognition
            std::vector<std::vector<double>> m_neuralWeights;
            std::vector<double> m_neuralBiases;
            
            // Adaptive learning parameters
            double m_learningRate = 0.01;
            double m_adaptationFactor = 1.05;
            std::chrono::time_point<std::chrono::steady_clock> m_lastLearningUpdate;

            // Updates the m_currentRiskLevel based on m_riskScore.
            void UpdateRiskLevel();
            // Adds an event to the history queue.
            void AddEventToHistory(AIEventType eventType);
            
            // Advanced AI helper methods
            double CalculatePatternSimilarity(const std::vector<AIEventType>& pattern1, 
                                            const std::vector<AIEventType>& pattern2);
            void UpdateNeuralWeights(const std::vector<double>& inputs, double targetOutput);
            double NeuralNetworkPredict(const std::vector<double>& inputs);
            std::vector<double> ExtractFeatures(const std::vector<AIEventType>& events);
            void AdaptiveWeightAdjustment();
            double CalculateTemporalRiskDecay(std::chrono::time_point<std::chrono::steady_clock> eventTime);
        };

    } // namespace Backend
} // namespace AetherVisor
