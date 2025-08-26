#include "AIController.h"
#include <algorithm> // For std::max

namespace AetherVisor {
    namespace Backend {

        // Constants for risk calculation. These would be tuned based on observation.
        constexpr double RISK_DECAY_FACTOR = 0.99; // Risk decays over time.
        constexpr double MAX_RISK_SCORE = 100.0;

        AIController& AIController::GetInstance() {
            static AIController instance;
            return instance;
        }

        void AIController::ReportEvent(AIEventType eventType) {
            double riskIncrease = 0.0;

            switch (eventType) {
                case AIEventType::INJECTION_ATTEMPT:
                    riskIncrease = 15.0;
                    break;
                case AIEventType::PAYLOAD_EXECUTED:
                    riskIncrease = 1.0;
                    break;
                case AIEventType::HOOK_CALLED:
                    riskIncrease = 0.5;
                    break;
                case AIEventType::MEMORY_PATCH_APPLIED:
                    riskIncrease = 10.0;
                    break;
                case AIEventType::MEMORY_READ:
                case AIEventType::MEMORY_WRITE:
                    riskIncrease = 2.0;
                    break;
                case AIEventType::NETWORK_PACKET_SENT:
                case AIEventType::NETWORK_PACKET_RECEIVED:
                    riskIncrease = 0.1;
                    break;
                case AIEventType::SUSPICIOUS_API_CALL:
                    riskIncrease = 25.0;
                    break;
                case AIEventType::SERVER_THROTTLING_DETECTED:
                    riskIncrease = 40.0;
                    break;
            }

            m_riskScore += riskIncrease;
            // Cap the risk score at the maximum value.
            if (m_riskScore > MAX_RISK_SCORE) {
                m_riskScore = MAX_RISK_SCORE;
            }

            UpdateRiskLevel();
        }

        RiskLevel AIController::GetCurrentRiskLevel() const {
            return m_currentRiskLevel;
        }

        bool AIController::ShouldPerformAction(RiskLevel requiredLevel) const {
            // Apply a decay to the risk score each time a decision is made.
            // This simulates risk decreasing over time when no new risky events occur.
            // Note: Making this const_cast is not ideal, but for this simulation it's okay.
            const_cast<AIController*>(this)->m_riskScore *= RISK_DECAY_FACTOR;
            const_cast<AIController*>(this)->UpdateRiskLevel();

            return static_cast<int>(m_currentRiskLevel) <= static_cast<int>(requiredLevel);
        }

        void AIController::UpdateRiskLevel() {
            if (m_riskScore >= 80.0) {
                m_currentRiskLevel = RiskLevel::CRITICAL;
            } else if (m_riskScore >= 50.0) {
                m_currentRiskLevel = RiskLevel::HIGH;
            } else if (m_riskScore >= 20.0) {
                m_currentRiskLevel = RiskLevel::MEDIUM;
            } else if (m_riskScore > 0.0) {
                m_currentRiskLevel = RiskLevel::LOW;
            } else {
                m_currentRiskLevel = RiskLevel::NONE;
            }
        }

    } // namespace Backend
} // namespace AetherVisor
