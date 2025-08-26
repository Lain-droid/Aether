#include "BehavioralCloner.h"

namespace AetherVisor {
    namespace ML {

        BehavioralCloner::BehavioralCloner(int input_size, int hidden_size, int output_size) {
            // Create a simple two-layer network with ReLU activation on the hidden layer.
            m_layers.emplace_back(input_size, hidden_size, &Matrix::relu);
            // The output layer often has no activation or a different one (e.g., linear).
            m_layers.emplace_back(hidden_size, output_size, nullptr);
        }

        std::pair<double, double> BehavioralCloner::GenerateMouseMovement(const GameState& state) {
            // 1. Convert the input game state into a 1xN matrix.
            Matrix inputs(1, 4);
            inputs.at(0, 0) = state.player_x;
            inputs.at(0, 1) = state.player_y;
            inputs.at(0, 2) = state.enemy_x;
            inputs.at(0, 3) = state.enemy_y;

            // 2. Perform the forward pass through all layers.
            Matrix current_output = inputs;
            for (auto& layer : m_layers) {
                current_output = layer.forward(current_output);
            }

            // 3. Extract the final output.
            // Assuming the output is a 1x2 matrix for (dx, dy).
            if (current_output.getCols() >= 2) {
                return { current_output.at(0, 0), current_output.at(0, 1) };
            }

            return { 0.0, 0.0 }; // Default fallback
        }

    } // namespace ML
} // namespace AetherVisor
