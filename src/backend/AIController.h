#pragma once

#include <string>

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
            FUNCTION_CALL_FAILED
        };

        enum class AIEventType {
            // Execution Events
            INJECTION_ATTEMPT,
            PAYLOAD_EXECUTED,
            HOOK_CALLED,

            // Memory Events
            MEMORY_PATCH_APPLIED,
            MEMORY_READ,
            MEMORY_WRITE,

            // Network Events
            NETWORK_PACKET_SENT,
            NETWORK_PACKET_RECEIVED,

            // Potential Detection Events
            SUSPICIOUS_API_CALL,
            SERVER_THROTTLING_DETECTED
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
            static const size_t MAX_HISTORY_SIZE = 10;

            // Updates the m_currentRiskLevel based on m_riskScore.
            void UpdateRiskLevel();
            // Adds an event to the history queue.
            void AddEventToHistory(AIEventType eventType);
        };

    } // namespace Backend
} // namespace AetherVisor
