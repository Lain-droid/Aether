#pragma once

#include "MLPrimitives.h"
#include <vector>
#include <chrono>
#include <utility>

namespace AetherVisor {
    namespace ML {

        // Advanced game state representation with temporal data
        struct GameState {
            double player_x, player_y, player_z;
            double enemy_x, enemy_y, enemy_z;
            double velocity_x, velocity_y, velocity_z;
            double camera_pitch, camera_yaw;
            double health, stamina;
            double time_since_last_action;
            std::vector<double> recent_actions; // History of recent inputs
            std::vector<double> environmental_factors; // Game-specific context
        };

        // Human-like input patterns
        struct HumanInputPattern {
            double reaction_time_ms;
            double movement_smoothness;
            double accuracy_variance;
            double input_timing_variance;
            double fatigue_factor;
            std::vector<double> preferred_key_sequences;
        };

        // Advanced human-like behavior cloning system
        class BehavioralCloner {
        public:
            // Constructor with advanced neural architecture
            BehavioralCloner(int input_size = 20, int hidden_size = 128, int output_size = 10);

            // Core behavior generation methods
            std::pair<double, double> GenerateMouseMovement(const GameState& state);
            std::vector<bool> GenerateKeyboardInput(const GameState& state);
            double GenerateReactionDelay(const GameState& state);
            
            // Advanced human-like behavior simulation
            void LearnFromHumanData(const std::vector<GameState>& states, 
                                  const std::vector<std::vector<double>>& humanActions);
            void AdaptToPlayStyle(const HumanInputPattern& pattern);
            
            // Anti-detection behavior
            void IntroduceHumanVariance();
            void SimulateFatigue(double sessionDuration);
            void GenerateNaturalPauses();
            
            // Pattern analysis and adaptation
            void AnalyzePlayerBehavior(const std::vector<GameState>& recentStates);
            HumanInputPattern DetectPlayerPattern(const std::vector<GameState>& playerHistory);
            
            // Advanced evasion behaviors
            void MimicPlayerStyle(const std::vector<GameState>& playerData);
            std::vector<double> GenerateDistractorActions(); // Generate believable "mistakes"
            
            // Context-aware decision making
            bool ShouldActAggressive(const GameState& state);
            bool ShouldActCautious(const GameState& state);
            double CalculateOptimalAimAccuracy(const GameState& state);

        private:
            std::vector<Layer> m_layers;
            std::vector<Layer> m_varianceNetwork; // For generating human-like variance
            
            // Human behavior modeling
            HumanInputPattern m_currentPattern;
            std::vector<double> m_reactionTimes;
            std::vector<double> m_accuracyHistory;
            std::vector<std::vector<double>> m_movementPatterns;
            
            // Session tracking for fatigue simulation
            std::chrono::time_point<std::chrono::steady_clock> m_sessionStart;
            double m_fatigueLevel = 0.0;
            int m_actionCount = 0;
            
            // Learning parameters
            double m_adaptationRate = 0.05;
            std::vector<std::pair<GameState, std::vector<double>>> m_trainingData;
            
            // Helper methods
            std::vector<double> ExtractBehavioralFeatures(const GameState& state);
            double ApplyHumanVariance(double baseValue, double varianceAmount);
            void UpdateFatigueLevel();
            std::vector<double> GenerateNaturalMovementCurve(double startX, double startY, 
                                                           double endX, double endY);
            bool ShouldMakeMistake();
            double CalculateContextualAccuracy(const GameState& state);
        };

    } // namespace ML
} // namespace AetherVisor
