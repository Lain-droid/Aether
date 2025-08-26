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


        // --- Layer Implementation ---
        Layer::Layer(int input_size, int output_size, std::function<Matrix(const Matrix&)> activation)
            : m_weights(input_size, output_size), m_biases(1, output_size), m_activation(activation) {
            // Initialize weights and biases with small random values.
            // This is just a placeholder for a real trained model.
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
            Matrix result = Matrix::multiply(inputs, m_weights);
            // TODO: Add biases here.

            // Apply the activation function if one was provided.
            if (m_activation) {
                return m_activation(result);
            }
            return result;
        }

    } // namespace ML
} // namespace AetherVisor
