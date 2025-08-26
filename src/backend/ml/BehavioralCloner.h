#pragma once

#include "MLPrimitives.h"
#include <vector>

namespace AetherVisor {
    namespace ML {

        // A placeholder struct for the input to our model.
        struct GameState {
            double player_x;
            double player_y;
            double enemy_x;
            double enemy_y;
        };

        // This class would use a trained model to generate human-like inputs.
        class BehavioralCloner {
        public:
            // Constructor sets up the neural network architecture.
            BehavioralCloner(int input_size, int hidden_size, int output_size);

            // Generates a mouse movement target based on the current game state.
            // Returns a pair of (delta_x, delta_y).
            std::pair<double, double> GenerateMouseMovement(const GameState& state);

            // Trains the network on a batch of data.
            void Train(const std::vector<GameState>& inputs, const std::vector<Matrix>& expected_outputs, double learning_rate, int epochs);

        private:
            std::vector<Layer> m_layers;
        };

    } // namespace ML
} // namespace AetherVisor
