#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <string>

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

        // Advanced activation functions
        class ActivationFunctions {
        public:
            static Matrix relu(const Matrix& m);
            static Matrix sigmoid(const Matrix& m);
            static Matrix tanh(const Matrix& m);
            static Matrix leaky_relu(const Matrix& m, double alpha = 0.01);
            static Matrix swish(const Matrix& m);
            static Matrix softmax(const Matrix& m);
            
            // Derivatives for backpropagation
            static Matrix relu_derivative(const Matrix& m);
            static Matrix sigmoid_derivative(const Matrix& m);
            static Matrix tanh_derivative(const Matrix& m);
        };

        // Advanced optimization algorithms
        enum class OptimizerType {
            SGD,
            ADAM,
            RMSPROP,
            ADAGRAD
        };

        class Optimizer {
        public:
            virtual ~Optimizer() = default;
            virtual void update(Matrix& weights, const Matrix& gradients) = 0;
            // Removed duplicate overload to avoid ambiguous redefinition
        };

        class AdamOptimizer : public Optimizer {
        public:
            AdamOptimizer(double learning_rate = 0.001, double beta1 = 0.9, double beta2 = 0.999, double epsilon = 1e-8);
            void update(Matrix& weights, const Matrix& gradients) override;
            // biases updated via same method; no separate signature
            
        private:
            double m_lr, m_beta1, m_beta2, m_epsilon;
            int m_timestep = 0;
            Matrix m_weights_momentum, m_weights_velocity;
            Matrix m_biases_momentum, m_biases_velocity;
        };

        // Advanced layer types
        class Layer {
        public:
            Layer(int input_size, int output_size, std::function<Matrix(const Matrix&)> activation);
            virtual ~Layer() = default;

            // Forward and backward passes
            virtual Matrix forward(const Matrix& inputs);
            virtual Matrix backward(const Matrix& grad_output);
            
            // Weight updates
            virtual void updateWeights(Optimizer& optimizer);
            
            // Regularization
            void setDropoutRate(double rate) { m_dropoutRate = rate; }
            void setBatchNormalization(bool enable) { m_batchNorm = enable; }
            
            // Access to weights and biases
            const Matrix& getWeights() const { return m_weights; }
            const Matrix& getBiases() const { return m_biases; }

        protected:
            Matrix m_weights;
            Matrix m_biases;
            Matrix m_lastInput; // Stored for backpropagation
            Matrix m_lastOutput; // Stored for backpropagation
            Matrix m_weightGradients;
            Matrix m_biasGradients;
            
            std::function<Matrix(const Matrix&)> m_activation;
            std::function<Matrix(const Matrix&)> m_activationDerivative;
            
            double m_dropoutRate = 0.0;
            bool m_batchNorm = false;
            bool m_training = true;
        };

        // Convolutional Layer for pattern recognition
        class ConvolutionalLayer : public Layer {
        public:
            ConvolutionalLayer(int input_channels, int output_channels, int kernel_size, int stride = 1, int padding = 0);
            Matrix forward(const Matrix& inputs) override;
            Matrix backward(const Matrix& grad_output) override;
            
        private:
            int m_inputChannels, m_outputChannels, m_kernelSize, m_stride, m_padding;
            std::vector<Matrix> m_kernels;
        };

        // LSTM Layer for sequential pattern learning
        class LSTMLayer : public Layer {
        public:
            LSTMLayer(int input_size, int hidden_size);
            Matrix forward(const Matrix& inputs) override;
            Matrix backward(const Matrix& grad_output) override;
            void reset(); // Reset hidden state
            
        private:
            int m_hiddenSize;
            Matrix m_hiddenState, m_cellState;
            Matrix m_forgetGate, m_inputGate, m_outputGate, m_candidateGate;
        };

        // Attention mechanism for advanced pattern recognition
        class AttentionLayer : public Layer {
        public:
            AttentionLayer(int d_model, int num_heads = 8);
            Matrix forward(const Matrix& query, const Matrix& key, const Matrix& value);
            Matrix multiHeadAttention(const Matrix& inputs);
            
        private:
            int m_dModel, m_numHeads, m_dKey;
            std::vector<Matrix> m_queryWeights, m_keyWeights, m_valueWeights;
            Matrix m_outputWeights;
        };

        // Advanced neural network class
        class NeuralNetwork {
        public:
            NeuralNetwork();
            ~NeuralNetwork() = default;

            // Network building
            void addLayer(std::unique_ptr<Layer> layer);
            void setOptimizer(std::unique_ptr<Optimizer> optimizer);
            void setLossFunction(std::function<double(const Matrix&, const Matrix&)> loss);
            
            // Training and inference
            Matrix forward(const Matrix& inputs);
            void backward(const Matrix& targets);
            double train(const std::vector<Matrix>& inputs, const std::vector<Matrix>& targets, int epochs = 100);
            Matrix predict(const Matrix& inputs);
            
            // Advanced training features
            void setRegularization(double l1 = 0.0, double l2 = 0.0);
            void setEarlyStopping(int patience = 10, double min_delta = 1e-4);
            void saveLearningCurve(const std::string& filename);
            
            // Model persistence
            bool saveModel(const std::string& filename);
            bool loadModel(const std::string& filename);
            
        private:
            std::vector<std::unique_ptr<Layer>> m_layers;
            std::unique_ptr<Optimizer> m_optimizer;
            std::function<double(const Matrix&, const Matrix&)> m_lossFunction;
            
            double m_l1Reg = 0.0, m_l2Reg = 0.0;
            int m_earlyStoppingPatience = 10;
            double m_earlyStoppingMinDelta = 1e-4;
            std::vector<double> m_lossHistory;
        };

    } // namespace ML
} // namespace AetherVisor
