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

        Matrix Matrix::add(const Matrix& a, const Matrix& b) {
            if (a.getRows() != b.getRows() || a.getCols() != b.getCols()) throw std::invalid_argument("Matrix dimensions must match for addition");
            Matrix result(a.getRows(), a.getCols());
            for (int i = 0; i < a.getRows(); ++i) {
                for (int j = 0; j < a.getCols(); ++j) {
                    result.at(i, j) = a.at(i, j) + b.at(i, j);
                }
            }
            return result;
        }

        Matrix Matrix::multiply(const Matrix& a, const Matrix& b) {
            if (a.getCols() != b.getRows()) throw std::invalid_argument(XorS("Matrix dimensions are not compatible for multiplication"));

            // Optimized version: Transpose 'b' to improve cache locality.
            Matrix b_transposed = b.transpose();
            Matrix result(a.getRows(), b.getCols());

            for (int i = 0; i < a.getRows(); ++i) {
                for (int j = 0; j < b.getCols(); ++j) {
                    double sum = 0.0;
                    for (int k = 0; k < a.getCols(); ++k) {
                        // Accessing both matrices in a more linear fashion.
                        sum += a.at(i, k) * b_transposed.at(j, k);
                    }
                    result.at(i, j) = sum;
                }
            }
            return result;
        }

        Matrix Matrix::transpose() const {
            Matrix result(m_cols, m_rows);
            for (int i = 0; i < m_rows; ++i) {
                for (int j = 0; j < m_cols; ++j) {
                    result.at(j, i) = this->at(i, j);
                }
            }
            return result;
        }

        Matrix Matrix::multiply_elementwise(const Matrix& a, const Matrix& b) {
             if (a.getRows() != b.getRows() || a.getCols() != b.getCols()) throw std::invalid_argument("Matrix dimensions must match for element-wise multiplication");
            Matrix result(a.getRows(), a.getCols());
            for (int i = 0; i < a.getRows(); ++i) {
                for (int j = 0; j < a.getCols(); ++j) {
                    result.at(i, j) = a.at(i, j) * b.at(i, j);
                }
            }
            return result;
        }

        void Matrix::subtract(const Matrix& other) {
            if (m_rows != other.getRows() || m_cols != other.getCols()) throw std::invalid_argument("Matrix dimensions must match for subtraction");
            for (int i = 0; i < m_rows; ++i) {
                for (int j = 0; j < m_cols(); ++j) {
                    this->at(i, j) -= other.at(i, j);
                }
            }
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


        // --- Layer Implementation ---
        Layer::Layer(int input_size, int output_size, std::function<Matrix(const Matrix&)> activation)
            : m_inputs(1, input_size), m_weights(input_size, output_size), m_biases(1, output_size), m_activation(activation) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> distrib(-0.5, 0.5);
            for (int i = 0; i < m_weights.getRows(); ++i) {
                for (int j = 0; j < m_weights.getCols(); ++j) {
                    m_weights.at(i, j) = distrib(gen);
                }
            }
        }

        Matrix Layer::forward(const Matrix& inputs) {
            m_inputs = inputs; // Store for backpropagation
            Matrix result = Matrix::multiply(inputs, m_weights);
            result = Matrix::add(result, m_biases);
            if (m_activation) {
                return m_activation(result);
            }
            return result;
        }

        Matrix Layer::backward(const Matrix& output_gradient, double learning_rate) {
            // This is a simplified backpropagation step.
            // A real implementation would also need the derivative of the activation function.
            Matrix weights_gradient = Matrix::multiply(m_inputs.transpose(), output_gradient);
            Matrix input_gradient = Matrix::multiply(output_gradient, m_weights.transpose());

            // Update weights and biases
            for(int i=0; i<weights_gradient.getRows(); ++i) {
                for(int j=0; j<weights_gradient.getCols(); ++j) {
                    weights_gradient.at(i, j) *= learning_rate;
                }
            }
            m_weights.subtract(weights_gradient);
            // Bias gradient is just the output gradient (simplified)
            // m_biases.subtract(output_gradient * learning_rate);

            return input_gradient;
        }

    } // namespace ML
} // namespace AetherVisor
