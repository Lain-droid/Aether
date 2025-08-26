#include "AIController.h"
#include <algorithm>

namespace AetherVisor {
    namespace Backend {

        constexpr double RISK_DECAY_FACTOR = 0.995;
        constexpr double MAX_RISK_SCORE = 100.0;
        constexpr double FEEDBACK_LEARNING_RATE = 1.20; // Increase risk weight by 20% on negative feedback.

        AIController& AIController::GetInstance() {
            static AIController instance;
            return instance;
        }

        AIController::AIController() {
            // Initialize the baseline risk weights for each event type.
            m_riskWeights = {
                { AIEventType::INJECTION_ATTEMPT, 15.0 },
                { AIEventType::PAYLOAD_EXECUTED, 1.0 },
                { AIEventType::HOOK_CALLED, 0.5 },
                { AIEventType::MEMORY_PATCH_APPLIED, 10.0 },
                { AIEventType::MEMORY_READ, 2.0 },
                { AIEventType::MEMORY_WRITE, 2.0 },
                { AIEventType::NETWORK_PACKET_SENT, 0.1 },
                { AIEventType::NETWORK_PACKET_RECEIVED, 0.1 },
                { AIEventType::SUSPICIOUS_API_CALL, 25.0 },
                { AIEventType::SERVER_THROTTLING_DETECTED, 40.0 }
            };
        }

        void AIController::ReportEvent(AIEventType eventType) {
            if (m_riskWeights.count(eventType)) {
                m_riskScore += m_riskWeights.at(eventType);
                if (m_riskScore > MAX_RISK_SCORE) {
                    m_riskScore = MAX_RISK_SCORE;
                }
                // Only add high-impact events to history for learning.
                if (m_riskWeights.at(eventType) > 5.0) {
                    AddEventToHistory(eventType);
                }
            }
            UpdateRiskLevel();
        }

        void AIController::ReportNegativeFeedback(FeedbackType type) {
            if (type == FeedbackType::NONE) return;

            // Learn from the feedback by increasing the risk weight of recent actions.
            for (const auto& eventType : m_recentEvents) {
                m_riskWeights[eventType] *= FEEDBACK_LEARNING_RATE;
            }
            // Clear history after learning from it.
            m_recentEvents.clear();
        }

        RiskLevel AIController::GetCurrentRiskLevel() const {
            return m_currentRiskLevel;
        }

        bool AIController::ShouldPerformAction(RiskLevel requiredLevel) const {
            const_cast<AIController*>(this)->m_riskScore *= RISK_DECAY_FACTOR;
            const_cast<AIController*>(this)->UpdateRiskLevel();
            return static_cast<int>(m_currentRiskLevel) <= static_cast<int>(requiredLevel);
        }

        void AIController::UpdateRiskLevel() {
            if (m_riskScore >= 80.0) m_currentRiskLevel = RiskLevel::CRITICAL;
            else if (m_riskScore >= 50.0) m_currentRiskLevel = RiskLevel::HIGH;
            else if (m_riskScore >= 20.0) m_currentRiskLevel = RiskLevel::MEDIUM;
            else if (m_riskScore > 0.0) m_currentRiskLevel = RiskLevel::LOW;
            else m_currentRiskLevel = RiskLevel::NONE;
        }

        void AIController::AddEventToHistory(AIEventType eventType) {
            m_recentEvents.push_back(eventType);
            if (m_recentEvents.size() > MAX_HISTORY_SIZE) {
                m_recentEvents.erase(m_recentEvents.begin());
            }
        }

    } // namespace Backend
} // namespace AetherVisor
