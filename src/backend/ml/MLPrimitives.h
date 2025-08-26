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

        private:
            int m_rows;
            int m_cols;
            std::vector<double> m_data;
        };

        // Represents one layer in a neural network.
        class Layer {
        public:
            Layer(int input_size, int output_size);

            // Performs the forward pass calculation for this layer.
            Matrix forward(const Matrix& inputs);

        private:
            Matrix m_weights;
            Matrix m_biases;
        };

    } // namespace ML
} // namespace AetherVisor
