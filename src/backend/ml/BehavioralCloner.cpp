#include "BehavioralCloner.h"

namespace AetherVisor {
    namespace ML {

        BehavioralCloner::BehavioralCloner(int input_size, int hidden_size, int output_size) {
            // Create a simple two-layer network with ReLU activation on the hidden layer.
            m_layers.emplace_back(input_size, hidden_size, &Matrix::relu);
            // The output layer often has no activation or a different one (e.g., linear).
            m_layers.emplace_back(hidden_size, output_size, nullptr);
        }

        void BehavioralCloner::Train(const std::vector<GameState>& inputs, const std::vector<Matrix>& expected_outputs, double learning_rate, int epochs) {
            for (int i = 0; i < epochs; ++i) {
                for (size_t j = 0; j < inputs.size(); ++j) {
                    // 1. Forward pass
                    Matrix current_output(1, 4);
                    current_output.at(0, 0) = inputs[j].player_x;
                    current_output.at(0, 1) = inputs[j].player_y;
                    current_output.at(0, 2) = inputs[j].enemy_x;
                    current_output.at(0, 3) = inputs[j].enemy_y;

                    std::vector<Matrix> layer_outputs;
                    layer_outputs.push_back(current_output);

                    for (auto& layer : m_layers) {
                        current_output = layer.forward(current_output);
                        layer_outputs.push_back(current_output);
                    }

                    // 2. Calculate error using the loss function's derivative
                    // This is the initial gradient that will be propagated backward.
                    Matrix gradient = Matrix::mean_squared_error_derivative(current_output, expected_outputs[j]);

                    // 3. Backward pass
                    for (int k = m_layers.size() - 1; k >= 0; --k) {
                        gradient = m_layers[k].backward(gradient, learning_rate);
                    }
                }
            }
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
