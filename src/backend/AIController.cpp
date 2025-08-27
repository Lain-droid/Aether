#include "AIController.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <numeric>
#include "security/XorStr.h"

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
                { AIEventType::SCRIPT_EXECUTION, 0.3 },
                { AIEventType::FUNCTION_OVERRIDE, 2.5 },
                { AIEventType::MEMORY_PATCH_APPLIED, 10.0 },
                { AIEventType::MEMORY_READ, 2.0 },
                { AIEventType::MEMORY_WRITE, 2.0 },
                { AIEventType::MEMORY_SCAN_EVASION, 8.0 },
                { AIEventType::PATTERN_OBFUSCATION, 5.0 },
                { AIEventType::NETWORK_PACKET_SENT, 0.1 },
                { AIEventType::NETWORK_PACKET_RECEIVED, 0.1 },
                { AIEventType::NETWORK_TIMING_ANOMALY, 15.0 },
                { AIEventType::BANDWIDTH_SPIKE, 12.0 },
                { AIEventType::SUSPICIOUS_API_CALL, 25.0 },
                { AIEventType::SERVER_THROTTLING_DETECTED, 40.0 },
                { AIEventType::ANTI_CHEAT_PROBE, 35.0 },
                { AIEventType::SIGNATURE_SCAN_ATTEMPT, 30.0 },
                { AIEventType::BEHAVIORAL_ANALYSIS_DETECTED, 45.0 },
                { AIEventType::NEURAL_PREDICTION, -2.0 },
                { AIEventType::PATTERN_LEARNING, -1.0 },
                { AIEventType::ADAPTIVE_BEHAVIOR_CHANGE, -3.0 }
            };

            // Initialize neural network for pattern recognition
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(-0.5, 0.5);

            // Create a 3-layer neural network: input -> hidden -> output
            const int inputSize = 10;  // Feature vector size
            const int hiddenSize = 20;
            const int outputSize = 1;  // Risk prediction

            // Initialize weights
            m_neuralWeights.resize(2);
            m_neuralWeights[0].resize(inputSize * hiddenSize);
            m_neuralWeights[1].resize(hiddenSize * outputSize);

            for (auto& weight : m_neuralWeights[0]) weight = dis(gen);
            for (auto& weight : m_neuralWeights[1]) weight = dis(gen);

            // Initialize biases
            m_neuralBiases.resize(hiddenSize + outputSize);
            for (auto& bias : m_neuralBiases) bias = dis(gen);

            m_lastLearningUpdate = std::chrono::steady_clock::now();
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

            // Record timing patterns for this event type
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastLearningUpdate).count();
            m_timingPatterns[eventType].push_back(static_cast<double>(duration));

            // Limit timing pattern history
            if (m_timingPatterns[eventType].size() > 20) {
                m_timingPatterns[eventType].erase(m_timingPatterns[eventType].begin());
            }

            // Trigger adaptive learning periodically
            if (m_recentEvents.size() % 10 == 0) {
                AdaptiveWeightAdjustment();
            }
        }

        // Advanced neural pattern prediction
        RiskLevel AIController::PredictFutureRisk(double timeHorizon) {
            if (m_recentEvents.size() < 5) return RiskLevel::NONE;

            // Extract features from recent events
            std::vector<double> features = ExtractFeatures(m_recentEvents);
            
            // Add time horizon as a feature
            features.push_back(timeHorizon);
            if (features.size() < 10) {
                features.resize(10, 0.0); // Pad with zeros
            }

            // Predict using neural network
            double predictedRisk = NeuralNetworkPredict(features);
            
            // Convert to risk level
            if (predictedRisk >= 0.8) return RiskLevel::CRITICAL;
            if (predictedRisk >= 0.6) return RiskLevel::HIGH;
            if (predictedRisk >= 0.4) return RiskLevel::MEDIUM;
            if (predictedRisk >= 0.2) return RiskLevel::LOW;
            return RiskLevel::NONE;
        }

        // Adaptive learning from behavioral patterns
        void AIController::LearnFromBehavioralPatterns(const std::vector<double>& behaviorData) {
            m_behavioralHistory.insert(m_behavioralHistory.end(), behaviorData.begin(), behaviorData.end());
            
            // Keep only recent behavioral data
            if (m_behavioralHistory.size() > PATTERN_ANALYSIS_WINDOW) {
                m_behavioralHistory.erase(m_behavioralHistory.begin(), 
                    m_behavioralHistory.begin() + (m_behavioralHistory.size() - PATTERN_ANALYSIS_WINDOW));
            }

            // Analyze patterns and update neural network
            if (m_behavioralHistory.size() >= 50) {
                std::vector<double> features = ExtractFeatures(m_recentEvents);
                double riskOutput = std::min(1.0, m_riskScore / MAX_RISK_SCORE);
                UpdateNeuralWeights(features, riskOutput);
            }
        }

        // Generate optimal timing for actions
        double AIController::GetOptimalActionTiming(AIEventType actionType) {
            if (m_timingPatterns.find(actionType) == m_timingPatterns.end()) {
                return 1000.0; // Default 1 second delay
            }

            auto& timings = m_timingPatterns[actionType];
            if (timings.empty()) return 1000.0;

            // Calculate average timing and add variance for unpredictability
            double avgTiming = std::accumulate(timings.begin(), timings.end(), 0.0) / timings.size();
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::normal_distribution<> d(avgTiming, avgTiming * 0.3); // 30% variance
            
            double optimalTiming = d(gen);
            return std::max(100.0, optimalTiming); // Minimum 100ms delay
        }

        // Advanced polymorphic decision making
        int AIController::GetRecommendedEvasionStrategy() {
            double currentRisk = m_riskScore / MAX_RISK_SCORE;
            
            // Use different strategies based on risk level and recent patterns
            if (currentRisk > 0.8) return 5; // Maximum evasion
            if (currentRisk > 0.6) return 4; // High evasion
            if (currentRisk > 0.4) return 3; // Medium evasion
            if (currentRisk > 0.2) return 2; // Low evasion
            return 1; // Minimal evasion
        }

        // Neural network-based anti-cheat detection
        double AIController::DetectAntiCheatActivity(const std::vector<AIEventType>& systemEvents) {
            if (systemEvents.empty()) return 0.0;

            std::vector<double> features = ExtractFeatures(systemEvents);
            if (features.size() < 10) features.resize(10, 0.0);

            // Use neural network to predict anti-cheat activity
            double confidence = NeuralNetworkPredict(features);
            
            // Boost confidence if we see specific suspicious patterns
            int suspiciousCount = 0;
            for (const auto& event : systemEvents) {
                if (event == AIEventType::ANTI_CHEAT_PROBE || 
                    event == AIEventType::SIGNATURE_SCAN_ATTEMPT ||
                    event == AIEventType::BEHAVIORAL_ANALYSIS_DETECTED) {
                    suspiciousCount++;
                }
            }
            
            confidence += suspiciousCount * 0.1; // Boost by 10% per suspicious event
            return std::min(1.0, confidence);
        }

        // Helper methods for neural network operations
        double AIController::NeuralNetworkPredict(const std::vector<double>& inputs) {
            if (inputs.size() != 10) return 0.5; // Invalid input size

            // Forward pass through hidden layer
            std::vector<double> hidden(20, 0.0);
            for (int i = 0; i < 20; ++i) {
                for (int j = 0; j < 10; ++j) {
                    hidden[i] += inputs[j] * m_neuralWeights[0][j * 20 + i];
                }
                hidden[i] += m_neuralBiases[i];
                hidden[i] = std::tanh(hidden[i]); // Tanh activation
            }

            // Forward pass through output layer
            double output = 0.0;
            for (int i = 0; i < 20; ++i) {
                output += hidden[i] * m_neuralWeights[1][i];
            }
            output += m_neuralBiases[20];
            
            // Sigmoid activation for output
            return 1.0 / (1.0 + std::exp(-output));
        }

        void AIController::UpdateNeuralWeights(const std::vector<double>& inputs, double targetOutput) {
            if (inputs.size() != 10) return;

            double predicted = NeuralNetworkPredict(inputs);
            double error = targetOutput - predicted;
            
            // Simple gradient descent update (simplified backpropagation)
            double learningRateAdjusted = m_learningRate * std::abs(error);
            
            // Update output layer weights
            for (int i = 0; i < 20; ++i) {
                m_neuralWeights[1][i] += learningRateAdjusted * error * inputs[i % 10];
            }
            
            // Update hidden layer weights (simplified)
            for (int i = 0; i < 200; ++i) {
                m_neuralWeights[0][i] += learningRateAdjusted * error * 0.1; // Simplified update
            }
        }

        std::vector<double> AIController::ExtractFeatures(const std::vector<AIEventType>& events) {
            std::vector<double> features(10, 0.0);
            
            if (events.empty()) return features;
            
            // Feature 0: Event frequency
            features[0] = static_cast<double>(events.size()) / MAX_HISTORY_SIZE;
            
            // Feature 1-5: Count of different event categories
            int executionEvents = 0, memoryEvents = 0, networkEvents = 0, detectionEvents = 0, aiEvents = 0;
            
            for (const auto& event : events) {
                if (static_cast<int>(event) <= 4) executionEvents++;
                else if (static_cast<int>(event) <= 9) memoryEvents++;
                else if (static_cast<int>(event) <= 13) networkEvents++;
                else if (static_cast<int>(event) <= 18) detectionEvents++;
                else aiEvents++;
            }
            
            features[1] = static_cast<double>(executionEvents) / events.size();
            features[2] = static_cast<double>(memoryEvents) / events.size();
            features[3] = static_cast<double>(networkEvents) / events.size();
            features[4] = static_cast<double>(detectionEvents) / events.size();
            features[5] = static_cast<double>(aiEvents) / events.size();
            
            // Feature 6: Pattern repetition
            int repeatedPatterns = 0;
            for (size_t i = 1; i < events.size(); ++i) {
                if (events[i] == events[i-1]) repeatedPatterns++;
            }
            features[6] = static_cast<double>(repeatedPatterns) / std::max(1, static_cast<int>(events.size()) - 1);
            
            // Feature 7: Temporal variance
            features[7] = m_riskScore / MAX_RISK_SCORE;
            
            // Feature 8: Recent high-risk events
            int recentHighRisk = 0;
            for (size_t i = std::max(0, static_cast<int>(events.size()) - 5); i < events.size(); ++i) {
                if (m_riskWeights.count(events[i]) && m_riskWeights.at(events[i]) > 10.0) {
                    recentHighRisk++;
                }
            }
            features[8] = static_cast<double>(recentHighRisk) / 5.0;
            
            // Feature 9: Current risk trend
            features[9] = static_cast<double>(m_currentRiskLevel) / 4.0;
            
            return features;
        }

        void AIController::AdaptiveWeightAdjustment() {
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::minutes>(now - m_lastLearningUpdate).count();
            
            if (timeSinceLastUpdate < 1) return; // Don't adjust too frequently
            
            // Adjust weights based on recent performance
            for (auto& [eventType, weight] : m_riskWeights) {
                // Count occurrences of this event type in recent history
                int count = std::count(m_recentEvents.begin(), m_recentEvents.end(), eventType);
                
                if (count > 3) {
                    // If an event is happening frequently but risk is low, reduce its weight
                    if (m_currentRiskLevel < RiskLevel::MEDIUM) {
                        weight *= 0.95;
                    }
                } else if (count == 0 && weight > 0) {
                    // If an event hasn't occurred, slightly increase its weight for sensitivity
                    weight *= 1.02;
                }
                
                // Keep weights within reasonable bounds
                weight = std::max(0.1, std::min(100.0, weight));
            }
            
            m_lastLearningUpdate = now;
        }

        double AIController::CalculateTemporalRiskDecay(std::chrono::time_point<std::chrono::steady_clock> eventTime) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - eventTime).count();
            
            // Exponential decay: risk decreases over time
            return std::exp(-elapsed / 30.0); // Half-life of 30 minutes
        }

    } // namespace Backend
} // namespace AetherVisor
