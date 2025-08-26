#include "MLPrimitives.h"
#include <stdexcept>
#include <random>

namespace AetherVisor {
    namespace ML {

        // --- Matrix Implementation ---
        Matrix::Matrix(int rows, int cols) : m_rows(rows), m_cols(cols) {
            m_data.resize(rows * cols, 0.0);
        }

        double& Matrix::at(int row, int col) {
            if (row >= m_rows || col >= m_cols) throw std::out_of_range("Matrix access out of bounds");
            return m_data[row * m_cols + col];
        }

        const double& Matrix::at(int row, int col) const {
            if (row >= m_rows || col >= m_cols) throw std::out_of_range("Matrix access out of bounds");
            return m_data[row * m_cols + col];
        }

        Matrix Matrix::multiply(const Matrix& a, const Matrix& b) {
            if (a.getCols() != b.getRows()) throw std::invalid_argument("Matrix dimensions are not compatible for multiplication");

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


        // --- Layer Implementation ---
        Layer::Layer(int input_size, int output_size)
            : m_weights(input_size, output_size), m_biases(1, output_size) {
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
            // In a real network, you'd add biases and apply an activation function here.
            return result;
        }

    } // namespace ML
} // namespace AetherVisor
