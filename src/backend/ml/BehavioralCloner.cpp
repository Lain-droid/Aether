#include "BehavioralCloner.h"
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <thread>

namespace AetherVisor {
    namespace ML {

        BehavioralCloner::BehavioralCloner(int input_size, int hidden_size, int output_size) {
            // Create advanced multi-layer network architecture
            m_layers.emplace_back(input_size, hidden_size, &Matrix::relu);
            m_layers.emplace_back(hidden_size, hidden_size / 2, &Matrix::relu);
            m_layers.emplace_back(hidden_size / 2, output_size, &Matrix::sigmoid);
            
            // Create variance network for human-like randomness
            m_varianceNetwork.emplace_back(input_size, hidden_size / 2, &Matrix::relu);
            m_varianceNetwork.emplace_back(hidden_size / 2, output_size, &Matrix::sigmoid);

            // Initialize human behavior pattern
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> dist_react(150.0, 250.0);
                std::uniform_real_distribution<> dist_move(0.8, 1.0);
                std::uniform_real_distribution<> dist_acc(0.1, 0.2);
                std::uniform_real_distribution<> dist_timing(0.05, 0.1);
                m_currentPattern = {
                    .reaction_time_ms = dist_react(gen),
                    .movement_smoothness = dist_move(gen),
                    .accuracy_variance = dist_acc(gen),
                    .input_timing_variance = dist_timing(gen),
                    .fatigue_factor = 0.0,
                    .preferred_key_sequences = {1.0, 0.8, 0.6, 0.9}
                };
            }

            m_sessionStart = std::chrono::steady_clock::now();
        }

        std::pair<double, double> BehavioralCloner::GenerateMouseMovement(const GameState& state) {
            // Extract comprehensive behavioral features
            std::vector<double> features = ExtractBehavioralFeatures(state);
            
            // Convert to matrix
            Matrix inputs(1, features.size());
            for (size_t i = 0; i < features.size(); ++i) {
                inputs.at(0, i) = features[i];
            }

            // Forward pass through main network
            Matrix current_output = inputs;
            for (auto& layer : m_layers) {
                current_output = layer.forward(current_output);
            }

            // Generate base movement
            double base_x = current_output.at(0, 0);
            double base_y = current_output.at(0, 1);

            // Apply human-like variance using variance network
            Matrix variance_output = inputs;
            for (auto& layer : m_varianceNetwork) {
                variance_output = layer.forward(variance_output);
            }

            double variance_x = variance_output.at(0, 0) * m_currentPattern.accuracy_variance;
            double variance_y = variance_output.at(0, 1) * m_currentPattern.accuracy_variance;

            // Apply fatigue effects
            UpdateFatigueLevel();
            base_x *= (1.0 - m_fatigueLevel * 0.3);
            base_y *= (1.0 - m_fatigueLevel * 0.3);

            // Generate natural movement curve if this is a large movement
            double movement_magnitude = sqrt(base_x * base_x + base_y * base_y);
            if (movement_magnitude > 100.0) {
                auto curve = GenerateNaturalMovementCurve(0, 0, base_x, base_y);
                base_x = curve.empty() ? base_x : curve[0];
                base_y = curve.size() > 1 ? curve[1] : base_y;
            }

            // Apply human variance
            double final_x = ApplyHumanVariance(base_x, variance_x);
            double final_y = ApplyHumanVariance(base_y, variance_y);

            // Occasional intentional "mistakes" for human-like behavior
            if (ShouldMakeMistake()) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist_err(-10, 10);
                final_x += dist_err(gen);
                final_y += dist_err(gen);
            }

            m_actionCount++;
            return { final_x, final_y };
        }

        std::vector<bool> BehavioralCloner::GenerateKeyboardInput(const GameState& state) {
            std::vector<bool> keyStates(10, false); // 10 common keys

            std::vector<double> features = ExtractBehavioralFeatures(state);
            
            // Determine action probabilities based on context
            bool isEnemyNear = sqrt(pow(state.enemy_x - state.player_x, 2) + 
                                  pow(state.enemy_y - state.player_y, 2)) < 50.0;
            
            bool shouldMove = state.velocity_x < 0.1 && state.velocity_y < 0.1;
            bool shouldAttack = isEnemyNear && state.health > 30.0;

            // Generate context-appropriate inputs with human-like timing
            if (shouldMove) {
                keyStates[0] = true; // W key
                {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dist_pct(0, 99);
                    if (dist_pct(gen) < 30) keyStates[1] = true; // A key
                    if (dist_pct(gen) < 30) keyStates[2] = true; // D key
                }
            }

            if (shouldAttack) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist_pct(0, 99);
                if (dist_pct(gen) < 80) {
                    keyStates[3] = true; // Attack key
                }
            }
                keyStates[3] = true; // Attack key
            }

            // Add human-like key combinations and preferences
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist_pct(0, 99);
                for (size_t i = 0; i < m_currentPattern.preferred_key_sequences.size() && i < keyStates.size(); ++i) {
                    if (dist_pct(gen) < static_cast<int>(m_currentPattern.preferred_key_sequences[i] * 100)) {
                        keyStates[i] = true;
                    }
                }
            }

            return keyStates;
        }

        double BehavioralCloner::GenerateReactionDelay(const GameState& state) {
            // Base reaction time with human-like variance
            double baseReaction = m_currentPattern.reaction_time_ms;
            
            // Adjust for context
            bool highPressure = state.health < 30.0 || 
                              sqrt(pow(state.enemy_x - state.player_x, 2) + 
                                 pow(state.enemy_y - state.player_y, 2)) < 20.0;
            
            if (highPressure) {
                baseReaction *= 0.8; // Faster reaction under pressure
            }

            // Apply fatigue
            baseReaction *= (1.0 + m_fatigueLevel * 0.5);

            // Add natural variance
            std::random_device rd;
            std::mt19937 gen(rd());
            std::normal_distribution<> d(baseReaction, baseReaction * m_currentPattern.input_timing_variance);
            
            return std::max(50.0, d(gen)); // Minimum 50ms reaction time
        }

        void BehavioralCloner::LearnFromHumanData(const std::vector<GameState>& states, 
                                                const std::vector<std::vector<double>>& humanActions) {
            if (states.size() != humanActions.size()) return;

            for (size_t i = 0; i < states.size(); ++i) {
                m_trainingData.emplace_back(states[i], humanActions[i]);
            }

            // Keep only recent training data
            if (m_trainingData.size() > 1000) {
                m_trainingData.erase(m_trainingData.begin(), 
                                   m_trainingData.begin() + (m_trainingData.size() - 1000));
            }

            // Analyze patterns in the human data
            AnalyzePlayerBehavior(states);
        }

        void BehavioralCloner::AdaptToPlayStyle(const HumanInputPattern& pattern) {
            // Gradually adapt to the new pattern
            m_currentPattern.reaction_time_ms = m_currentPattern.reaction_time_ms * (1.0 - m_adaptationRate) + 
                                              pattern.reaction_time_ms * m_adaptationRate;
            
            m_currentPattern.movement_smoothness = m_currentPattern.movement_smoothness * (1.0 - m_adaptationRate) + 
                                                 pattern.movement_smoothness * m_adaptationRate;
            
            m_currentPattern.accuracy_variance = m_currentPattern.accuracy_variance * (1.0 - m_adaptationRate) + 
                                               pattern.accuracy_variance * m_adaptationRate;

            // Adapt preferred sequences
            for (size_t i = 0; i < std::min(m_currentPattern.preferred_key_sequences.size(), 
                                          pattern.preferred_key_sequences.size()); ++i) {
                m_currentPattern.preferred_key_sequences[i] = 
                    m_currentPattern.preferred_key_sequences[i] * (1.0 - m_adaptationRate) + 
                    pattern.preferred_key_sequences[i] * m_adaptationRate;
            }
        }

        void BehavioralCloner::IntroduceHumanVariance() {
            // Randomly adjust parameters to simulate human inconsistency
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(-0.1, 0.1);

            m_currentPattern.reaction_time_ms *= (1.0 + dis(gen));
            m_currentPattern.accuracy_variance *= (1.0 + dis(gen));
            m_currentPattern.movement_smoothness *= (1.0 + dis(gen));

            // Clamp values to reasonable ranges
            m_currentPattern.reaction_time_ms = std::max(100.0, std::min(500.0, m_currentPattern.reaction_time_ms));
            m_currentPattern.accuracy_variance = std::max(0.05, std::min(0.3, m_currentPattern.accuracy_variance));
            m_currentPattern.movement_smoothness = std::max(0.5, std::min(1.0, m_currentPattern.movement_smoothness));
        }

        void BehavioralCloner::SimulateFatigue(double sessionDuration) {
            // Fatigue increases over time, affecting accuracy and reaction time
            m_fatigueLevel = std::min(1.0, sessionDuration / 7200.0); // Max fatigue after 2 hours
            
            // Add periodic breaks to simulate natural human behavior
            if (sessionDuration > 1800) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist_thousand(0, 999);
                if (dist_thousand(gen) < 1) {
                    GenerateNaturalPauses();
                }
            }
        }

        void BehavioralCloner::GenerateNaturalPauses() {
            // Simulate natural micro-pauses in human behavior
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist_ms(100, 399);
                std::this_thread::sleep_for(std::chrono::milliseconds(dist_ms(gen)));
            }
        }

        // Helper method implementations
        std::vector<double> BehavioralCloner::ExtractBehavioralFeatures(const GameState& state) {
            std::vector<double> features;
            
            // Position and movement features
            features.push_back(state.player_x / 1000.0); // Normalize
            features.push_back(state.player_y / 1000.0);
            features.push_back(state.player_z / 1000.0);
            features.push_back(state.velocity_x / 100.0);
            features.push_back(state.velocity_y / 100.0);
            features.push_back(state.velocity_z / 100.0);

            // Enemy relative position
            double rel_x = (state.enemy_x - state.player_x) / 1000.0;
            double rel_y = (state.enemy_y - state.player_y) / 1000.0;
            double rel_z = (state.enemy_z - state.player_z) / 1000.0;
            features.push_back(rel_x);
            features.push_back(rel_y);
            features.push_back(rel_z);

            // Distance to enemy
            double distance = sqrt(rel_x*rel_x + rel_y*rel_y + rel_z*rel_z);
            features.push_back(distance);

            // Camera and orientation
            features.push_back(state.camera_pitch / 90.0); // Normalize to [-1, 1]
            features.push_back(state.camera_yaw / 180.0);

            // Health and status
            features.push_back(state.health / 100.0);
            features.push_back(state.stamina / 100.0);

            // Temporal features
            features.push_back(state.time_since_last_action / 5000.0); // Normalize to 5 seconds
            features.push_back(m_fatigueLevel);

            // Add recent action context
            if (!state.recent_actions.empty()) {
                for (size_t i = 0; i < std::min(size_t(3), state.recent_actions.size()); ++i) {
                    features.push_back(state.recent_actions[i]);
                }
            }

            // Pad to consistent size
            while (features.size() < 20) {
                features.push_back(0.0);
            }
            features.resize(20); // Ensure exactly 20 features

            return features;
        }

        double BehavioralCloner::ApplyHumanVariance(double baseValue, double varianceAmount) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::normal_distribution<> d(0.0, varianceAmount);
            
            return baseValue + d(gen);
        }

        void BehavioralCloner::UpdateFatigueLevel() {
            auto now = std::chrono::steady_clock::now();
            auto sessionDuration = std::chrono::duration_cast<std::chrono::minutes>(now - m_sessionStart).count();
            
            m_fatigueLevel = std::min(1.0, sessionDuration / 120.0); // Max fatigue after 2 hours
            
            // Add micro-fatigue based on action count
            double actionFatigue = std::min(0.3, m_actionCount / 10000.0);
            m_fatigueLevel = std::min(1.0, m_fatigueLevel + actionFatigue);
        }

        std::vector<double> BehavioralCloner::GenerateNaturalMovementCurve(double startX, double startY, 
                                                                         double endX, double endY) {
            std::vector<double> curve;
            
            // Generate Bezier-like curve for natural mouse movement
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist_err(-10, 10);
                double midX = (startX + endX) / 2.0 + dist_err(gen);
                double midY = (startY + endY) / 2.0 + dist_err(gen);
                // Simplified curve - return adjusted end point
                curve.push_back(endX + (midX - endX) * 0.1);
                curve.push_back(endY + (midY - endY) * 0.1);
                return curve;
            }
            return curve; // Unreachable, retained for structure
        }

        bool BehavioralCloner::ShouldMakeMistake() {
            // Probability of making a mistake increases with fatigue
            double mistakeChance = 0.01 + m_fatigueLevel * 0.05; // 1-6% chance
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist_thousand(0, 999);
            return dist_thousand(gen) < static_cast<int>(mistakeChance * 1000);
        }

        double BehavioralCloner::CalculateContextualAccuracy(const GameState& state) {
            double baseAccuracy = 0.85; // 85% base accuracy
            
            // Adjust for distance
            double distance = sqrt(pow(state.enemy_x - state.player_x, 2) + 
                                 pow(state.enemy_y - state.player_y, 2));
            double distanceModifier = std::max(0.5, 1.0 - (distance / 200.0));
            
            // Adjust for health (stress)
            double healthModifier = state.health / 100.0;
            
            // Adjust for fatigue
            double fatigueModifier = 1.0 - m_fatigueLevel * 0.4;
            
            return baseAccuracy * distanceModifier * healthModifier * fatigueModifier;
        }

        void BehavioralCloner::AnalyzePlayerBehavior(const std::vector<GameState>& recentStates) {
            if (recentStates.size() < 10) return;

            // Analyze reaction times
            std::vector<double> reactionTimes;
            for (size_t i = 1; i < recentStates.size(); ++i) {
                double timeDiff = recentStates[i].time_since_last_action;
                if (timeDiff > 0 && timeDiff < 2000) { // Valid reaction time
                    reactionTimes.push_back(timeDiff);
                }
            }

            if (!reactionTimes.empty()) {
                double avgReaction = std::accumulate(reactionTimes.begin(), reactionTimes.end(), 0.0) / reactionTimes.size();
                m_currentPattern.reaction_time_ms = avgReaction * 0.3 + m_currentPattern.reaction_time_ms * 0.7; // Blend
            }

            // Analyze movement patterns and update accordingly
            // This is a simplified version - real implementation would be much more sophisticated
        }

        HumanInputPattern BehavioralCloner::DetectPlayerPattern(const std::vector<GameState>& playerHistory) {
            HumanInputPattern detectedPattern = m_currentPattern;
            
            if (playerHistory.size() < 20) return detectedPattern;

            // Analyze the player's actual behavior patterns
            std::vector<double> movementSpeeds;
            std::vector<double> accuracyScores;
            
            for (const auto& state : playerHistory) {
                double speed = sqrt(state.velocity_x * state.velocity_x + state.velocity_y * state.velocity_y);
                movementSpeeds.push_back(speed);
                
                // Calculate implied accuracy based on aim and enemy position
                double distance = sqrt(pow(state.enemy_x - state.player_x, 2) + pow(state.enemy_y - state.player_y, 2));
                if (distance > 0) {
                    accuracyScores.push_back(1.0 / (1.0 + distance / 100.0)); // Simplified accuracy metric
                }
            }

            // Update pattern based on analysis
            if (!movementSpeeds.empty()) {
                double avgSpeed = std::accumulate(movementSpeeds.begin(), movementSpeeds.end(), 0.0) / movementSpeeds.size();
                detectedPattern.movement_smoothness = std::min(1.0, avgSpeed / 50.0);
            }

            if (!accuracyScores.empty()) {
                double avgAccuracy = std::accumulate(accuracyScores.begin(), accuracyScores.end(), 0.0) / accuracyScores.size();
                detectedPattern.accuracy_variance = std::max(0.05, 1.0 - avgAccuracy);
            }

            return detectedPattern;
        }

    } // namespace ML
} // namespace AetherVisor
