#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "MLPrimitives.h"
#include "../security/XorStr.h"
#include <stdexcept>
#include <random>
#include <cmath> // For std::exp

namespace AetherVisor {
    namespace ML {

        // --- Matrix Implementation ---
        Matrix::Matrix(int rows, int cols) : m_rows(rows), m_cols(cols) {
            m_data.resize(rows * cols, 0.0);
        }

        double& Matrix::at(int row, int col) {
            if (row >= m_rows || col >= m_cols) throw std::out_of_range(XorS("Matrix access out of bounds"));
            return m_data[row * m_cols + col];
        }

        const double& Matrix::at(int row, int col) const {
            if (row >= m_rows || col >= m_cols) throw std::out_of_range(XorS("Matrix access out of bounds"));
            return m_data[row * m_cols + col];
        }

        Matrix Matrix::multiply(const Matrix& a, const Matrix& b) {
            if (a.getCols() != b.getRows()) throw std::invalid_argument(XorS("Matrix dimensions are not compatible for multiplication"));

            Matrix result(a.getRows(), b.getCols());
            for (int i = 0; i < a.getRows(); ++i) {
                for (int j = 0; j < b.getCols(); ++j) {
                    for (int k = 0; k < a.getCols(); ++k) {
                        result.at(i, j) += a.at(i, k) * b.at(k, j);
                    }
                }
            }
            return result;
        }

        Matrix Matrix::relu(const Matrix& m) {
            Matrix result(m.getRows(), m.getCols());
            for (int i = 0; i < m.getRows(); ++i) {
                for (int j = 0; j < m.getCols(); ++j) {
                    result.at(i, j) = std::max(0.0, m.at(i, j));
                }
            }
            return result;
        }

        Matrix Matrix::sigmoid(const Matrix& m) {
            Matrix result(m.getRows(), m.getCols());
            for (int i = 0; i < m.getRows(); ++i) {
                for (int j = 0; j < m.getCols(); ++j) {
                    result.at(i, j) = 1.0 / (1.0 + std::exp(-m.at(i, j)));
                }
            }
            return result;
        }

        // Advanced Activation Functions Implementation
        Matrix ActivationFunctions::relu(const Matrix& m) {
            return Matrix::relu(m);
        }

        Matrix ActivationFunctions::sigmoid(const Matrix& m) {
            return Matrix::sigmoid(m);
        }

        Matrix ActivationFunctions::tanh(const Matrix& m) {
            Matrix result(m.getRows(), m.getCols());
            for (int i = 0; i < m.getRows(); ++i) {
                for (int j = 0; j < m.getCols(); ++j) {
                    result.at(i, j) = std::tanh(m.at(i, j));
                }
            }
            return result;
        }

        Matrix ActivationFunctions::leaky_relu(const Matrix& m, double alpha) {
            Matrix result(m.getRows(), m.getCols());
            for (int i = 0; i < m.getRows(); ++i) {
                for (int j = 0; j < m.getCols(); ++j) {
                    double val = m.at(i, j);
                    result.at(i, j) = val > 0 ? val : alpha * val;
                }
            }
            return result;
        }

        Matrix ActivationFunctions::swish(const Matrix& m) {
            Matrix result(m.getRows(), m.getCols());
            for (int i = 0; i < m.getRows(); ++i) {
                for (int j = 0; j < m.getCols(); ++j) {
                    double val = m.at(i, j);
                    result.at(i, j) = val / (1.0 + std::exp(-val));
                }
            }
            return result;
        }

        Matrix ActivationFunctions::softmax(const Matrix& m) {
            Matrix result(m.getRows(), m.getCols());
            for (int i = 0; i < m.getRows(); ++i) {
                double sum = 0.0;
                // Find max for numerical stability
                double max_val = m.at(i, 0);
                for (int j = 1; j < m.getCols(); ++j) {
                    max_val = std::max(max_val, m.at(i, j));
                }
                
                // Calculate exp(x - max) and sum
                for (int j = 0; j < m.getCols(); ++j) {
                    double exp_val = std::exp(m.at(i, j) - max_val);
                    result.at(i, j) = exp_val;
                    sum += exp_val;
                }
                
                // Normalize
                for (int j = 0; j < m.getCols(); ++j) {
                    result.at(i, j) /= sum;
                }
            }
            return result;
        }

        Matrix ActivationFunctions::relu_derivative(const Matrix& m) {
            Matrix result(m.getRows(), m.getCols());
            for (int i = 0; i < m.getRows(); ++i) {
                for (int j = 0; j < m.getCols(); ++j) {
                    result.at(i, j) = m.at(i, j) > 0 ? 1.0 : 0.0;
                }
            }
            return result;
        }

        Matrix ActivationFunctions::sigmoid_derivative(const Matrix& m) {
            Matrix result(m.getRows(), m.getCols());
            for (int i = 0; i < m.getRows(); ++i) {
                for (int j = 0; j < m.getCols(); ++j) {
                    double sig = 1.0 / (1.0 + std::exp(-m.at(i, j)));
                    result.at(i, j) = sig * (1.0 - sig);
                }
            }
            return result;
        }

        Matrix ActivationFunctions::tanh_derivative(const Matrix& m) {
            Matrix result(m.getRows(), m.getCols());
            for (int i = 0; i < m.getRows(); ++i) {
                for (int j = 0; j < m.getCols(); ++j) {
                    double tanh_val = std::tanh(m.at(i, j));
                    result.at(i, j) = 1.0 - tanh_val * tanh_val;
                }
            }
            return result;
        }

        // Adam Optimizer Implementation
        AdamOptimizer::AdamOptimizer(double learning_rate, double beta1, double beta2, double epsilon)
            : m_lr(learning_rate), m_beta1(beta1), m_beta2(beta2), m_epsilon(epsilon),
              m_weights_momentum(0,0), m_weights_velocity(0,0),
              m_biases_momentum(0,0), m_biases_velocity(0,0) {}

        void AdamOptimizer::update(Matrix& weights, const Matrix& gradients) {
            if (m_weights_momentum.getRows() == 0) {
                m_weights_momentum = Matrix(weights.getRows(), weights.getCols());
                m_weights_velocity = Matrix(weights.getRows(), weights.getCols());
            }

            m_timestep++;
            
            // Update momentum and velocity
            for (int i = 0; i < weights.getRows(); ++i) {
                for (int j = 0; j < weights.getCols(); ++j) {
                    double grad = gradients.at(i, j);
                    
                    // Update momentum (first moment)
                    m_weights_momentum.at(i, j) = m_beta1 * m_weights_momentum.at(i, j) + (1.0 - m_beta1) * grad;
                    
                    // Update velocity (second moment)
                    m_weights_velocity.at(i, j) = m_beta2 * m_weights_velocity.at(i, j) + (1.0 - m_beta2) * grad * grad;
                    
                    // Bias correction
                    double momentum_corrected = m_weights_momentum.at(i, j) / (1.0 - std::pow(m_beta1, m_timestep));
                    double velocity_corrected = m_weights_velocity.at(i, j) / (1.0 - std::pow(m_beta2, m_timestep));
                    
                    // Update weights
                    weights.at(i, j) -= m_lr * momentum_corrected / (std::sqrt(velocity_corrected) + m_epsilon);
                }
            }
        }

        // single update() handles both weights and biases via caller


        // --- Enhanced Layer Implementation ---
        Layer::Layer(int input_size, int output_size, std::function<Matrix(const Matrix&)> activation)
            : m_weights(input_size, output_size), m_biases(1, output_size), m_activation(activation),
              m_lastInput(1, input_size), m_lastOutput(1, output_size),
              m_weightGradients(input_size, output_size), m_biasGradients(1, output_size) {
            
            // Xavier/Glorot initialization for better convergence
            std::random_device rd;
            std::mt19937 gen(rd());
            double variance = 2.0 / (input_size + output_size);
            std::normal_distribution<> distrib(0.0, std::sqrt(variance));

            for (int i = 0; i < m_weights.getRows(); ++i) {
                for (int j = 0; j < m_weights.getCols(); ++j) {
                    m_weights.at(i, j) = distrib(gen);
                }
            }

            // Initialize biases to small values
            for (int j = 0; j < m_biases.getCols(); ++j) {
                m_biases.at(0, j) = 0.01;
            }

            // Set activation derivative based on activation function
            if (activation.target<Matrix(*)(const Matrix&)>() && *activation.target<Matrix(*)(const Matrix&)>() == &Matrix::relu) {
                m_activationDerivative = &ActivationFunctions::relu_derivative;
            } else if (activation.target<Matrix(*)(const Matrix&)>() && *activation.target<Matrix(*)(const Matrix&)>() == &Matrix::sigmoid) {
                m_activationDerivative = &ActivationFunctions::sigmoid_derivative;
            } else if (activation.target<Matrix(*)(const Matrix&)>() && *activation.target<Matrix(*)(const Matrix&)>() == &ActivationFunctions::tanh) {
                m_activationDerivative = &ActivationFunctions::tanh_derivative;
            }
        }

        Matrix Layer::forward(const Matrix& inputs) {
            // Store input for backpropagation
            m_lastInput = inputs;
            
            // Linear transformation: W*X + b
            Matrix result = Matrix::multiply(inputs, m_weights);
            
            // Add biases
            for (int i = 0; i < result.getRows(); ++i) {
                for (int j = 0; j < result.getCols(); ++j) {
                    result.at(i, j) += m_biases.at(0, j);
                }
            }

            // Apply dropout during training
            if (m_training && m_dropoutRate > 0.0) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> dis(0.0, 1.0);
                
                for (int i = 0; i < result.getRows(); ++i) {
                    for (int j = 0; j < result.getCols(); ++j) {
                        if (dis(gen) < m_dropoutRate) {
                            result.at(i, j) = 0.0;
                        } else {
                            result.at(i, j) /= (1.0 - m_dropoutRate); // Scale remaining values
                        }
                    }
                }
            }

            // Apply activation function
            if (m_activation) {
                result = m_activation(result);
            }

            // Store output for backpropagation
            m_lastOutput = result;
            return result;
        }

        Matrix Layer::backward(const Matrix& grad_output) {
            // Compute gradients w.r.t. activation
            Matrix activation_grad = grad_output;
            if (m_activationDerivative) {
                Matrix pre_activation = Matrix::multiply(m_lastInput, m_weights);
                for (int i = 0; i < pre_activation.getRows(); ++i) {
                    for (int j = 0; j < pre_activation.getCols(); ++j) {
                        pre_activation.at(i, j) += m_biases.at(0, j);
                    }
                }
                Matrix derivative = m_activationDerivative(pre_activation);
                
                // Element-wise multiplication
                for (int i = 0; i < activation_grad.getRows(); ++i) {
                    for (int j = 0; j < activation_grad.getCols(); ++j) {
                        activation_grad.at(i, j) *= derivative.at(i, j);
                    }
                }
            }

            // Compute weight gradients: X^T * grad_output
            Matrix input_transposed(m_lastInput.getCols(), m_lastInput.getRows());
            for (int i = 0; i < m_lastInput.getRows(); ++i) {
                for (int j = 0; j < m_lastInput.getCols(); ++j) {
                    input_transposed.at(j, i) = m_lastInput.at(i, j);
                }
            }
            m_weightGradients = Matrix::multiply(input_transposed, activation_grad);

            // Compute bias gradients: sum of grad_output across batch dimension
            for (int j = 0; j < m_biasGradients.getCols(); ++j) {
                double sum = 0.0;
                for (int i = 0; i < activation_grad.getRows(); ++i) {
                    sum += activation_grad.at(i, j);
                }
                m_biasGradients.at(0, j) = sum;
            }

            // Compute gradients w.r.t. input: grad_output * W^T
            Matrix weights_transposed(m_weights.getCols(), m_weights.getRows());
            for (int i = 0; i < m_weights.getRows(); ++i) {
                for (int j = 0; j < m_weights.getCols(); ++j) {
                    weights_transposed.at(j, i) = m_weights.at(i, j);
                }
            }
            
            return Matrix::multiply(activation_grad, weights_transposed);
        }

        void Layer::updateWeights(Optimizer& optimizer) {
            optimizer.update(m_weights, m_weightGradients);
            optimizer.update(m_biases, m_biasGradients);
        }

        // Neural Network Implementation
        NeuralNetwork::NeuralNetwork() {
            // Set default loss function (Mean Squared Error)
            m_lossFunction = [](const Matrix& predicted, const Matrix& actual) {
                double sum = 0.0;
                int count = predicted.getRows() * predicted.getCols();
                
                for (int i = 0; i < predicted.getRows(); ++i) {
                    for (int j = 0; j < predicted.getCols(); ++j) {
                        double diff = predicted.at(i, j) - actual.at(i, j);
                        sum += diff * diff;
                    }
                }
                return sum / count;
            };
        }

        void NeuralNetwork::addLayer(std::unique_ptr<Layer> layer) {
            m_layers.push_back(std::move(layer));
        }

        void NeuralNetwork::setOptimizer(std::unique_ptr<Optimizer> optimizer) {
            m_optimizer = std::move(optimizer);
        }

        void NeuralNetwork::setLossFunction(std::function<double(const Matrix&, const Matrix&)> loss) {
            m_lossFunction = loss;
        }

        Matrix NeuralNetwork::forward(const Matrix& inputs) {
            Matrix current = inputs;
            for (auto& layer : m_layers) {
                current = layer->forward(current);
            }
            return current;
        }

        void NeuralNetwork::backward(const Matrix& targets) {
            if (m_layers.empty()) return;

            // Compute loss gradient (simplified MSE gradient)
            Matrix grad_output(targets.getRows(), targets.getCols());
            Matrix predicted = m_layers.back()->getLastOutput();
            
            for (int i = 0; i < targets.getRows(); ++i) {
                for (int j = 0; j < targets.getCols(); ++j) {
                    grad_output.at(i, j) = 2.0 * (predicted.at(i, j) - targets.at(i, j)) / (targets.getRows() * targets.getCols());
                }
            }

            // Backpropagate through layers
            Matrix current_grad = grad_output;
            for (int i = m_layers.size() - 1; i >= 0; --i) {
                current_grad = m_layers[i]->backward(current_grad);
            }

            // Update weights if optimizer is set
            if (m_optimizer) {
                for (auto& layer : m_layers) {
                    layer->updateWeights(*m_optimizer);
                }
            }
        }

        double NeuralNetwork::train(const std::vector<Matrix>& inputs, const std::vector<Matrix>& targets, int epochs) {
            if (inputs.size() != targets.size()) return -1.0;
            
            double final_loss = 0.0;
            
            for (int epoch = 0; epoch < epochs; ++epoch) {
                double epoch_loss = 0.0;
                
                for (size_t i = 0; i < inputs.size(); ++i) {
                    // Forward pass
                    Matrix predicted = forward(inputs[i]);
                    
                    // Compute loss
                    double loss = m_lossFunction(predicted, targets[i]);
                    epoch_loss += loss;
                    
                    // Backward pass
                    backward(targets[i]);
                }
                
                epoch_loss /= inputs.size();
                m_lossHistory.push_back(epoch_loss);
                final_loss = epoch_loss;
                
                // Early stopping check
                if (m_lossHistory.size() > m_earlyStoppingPatience) {
                    bool should_stop = true;
                    for (int j = 1; j <= m_earlyStoppingPatience; ++j) {
                        if (m_lossHistory[m_lossHistory.size() - j] - 
                            m_lossHistory[m_lossHistory.size() - j - 1] < -m_earlyStoppingMinDelta) {
                            should_stop = false;
                            break;
                        }
                    }
                    if (should_stop) break;
                }
            }
            
            return final_loss;
        }

        Matrix NeuralNetwork::predict(const Matrix& inputs) {
            return forward(inputs);
        }

    } // namespace ML
} // namespace AetherVisor
