#pragma once

#include <vector>
#include <functional>

namespace AetherVisor {
    namespace ML {

        // A simple Matrix class for basic neural network operations.
        class Matrix {
        public:
            Matrix(int rows, int cols);

            // Accessor methods
            double& at(int row, int col);
            const double& at(int row, int col) const;
            int getRows() const { return m_rows; }
            int getCols() const { return m_cols; }

            // Static method for matrix multiplication
            static Matrix multiply(const Matrix& a, const Matrix& b);

            // Static methods for activation functions
            static Matrix relu(const Matrix& m);
            static Matrix sigmoid(const Matrix& m);

        private:
            int m_rows;
            int m_cols;
            std::vector<double> m_data;
        };

        // Represents one layer in a neural network.
        class Layer {
        public:
            // The activation function is now passed to the constructor.
            Layer(int input_size, int output_size, std::function<Matrix(const Matrix&)> activation);

            // Performs the forward pass calculation for this layer.
            Matrix forward(const Matrix& inputs);

        private:
            Matrix m_weights;
            Matrix m_biases;
            std::function<Matrix(const Matrix&)> m_activation;
        };

    } // namespace ML
} // namespace AetherVisor
