#include "Compiler.h"
#include <sstream>
#include <stack>
#include <queue>
#include <map>
#include <stdexcept>

namespace AetherVisor {
    namespace VM {

        Compiler::Compiler() {}

        // Helper to get operator precedence
        int GetPrecedence(char op) {
            switch (op) {
                case '+': case '-': return 1;
                case '*': case '/': return 2;
                default: return 0;
            }
        }

        // Helper to convert RPN queue to bytecode
        std::vector<uint8_t> RpnToBytecode(std::queue<std::string>& rpnQueue) {
            std::vector<uint8_t> bytecode;
            while (!rpnQueue.empty()) {
                std::string token = rpnQueue.front();
                rpnQueue.pop();

                if (isdigit(token[0])) {
                    bytecode.push_back(static_cast<uint8_t>(VMOpcode::PUSH_INT));
                    int32_t val = std::stoi(token);
                    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&val);
                    bytecode.insert(bytecode.end(), bytes, bytes + sizeof(int32_t));
                } else {
                    switch (token[0]) {
                        case '+': bytecode.push_back(static_cast<uint8_t>(VMOpcode::ADD)); break;
                        case '-': bytecode.push_back(static_cast<uint8_t>(VMOpcode::SUB)); break;
                        case '*': bytecode.push_back(static_cast<uint8_t>(VMOpcode::MUL)); break;
                        case '/': bytecode.push_back(static_cast<uint8_t>(VMOpcode::DIV)); break;
                    }
                }
            }
            return bytecode;
        }

        // Tokenizer to handle multi-digit numbers
        std::vector<std::string> Tokenize(const std::string& expression) {
            std::vector<std::string> tokens;
            std::string current_number;

            for (char c : expression) {
                if (isspace(c)) continue;
                if (isdigit(c)) {
                    current_number += c;
                } else {
                    if (!current_number.empty()) {
                        tokens.push_back(current_number);
                        current_number.clear();
                    }
                    tokens.push_back(std::string(1, c));
                }
            }
            if (!current_number.empty()) {
                tokens.push_back(current_number);
            }
            return tokens;
        }

        // Implementation of Shunting-yard algorithm
        std::vector<uint8_t> Compiler::Compile(const std::string& expression) {
            // NOTE: This is still a simplified compiler. It doesn't support native calls
            // within complex expressions. This just demonstrates compiling a single call.
            if (expression == "MyNativeFunc()") {
                std::vector<uint8_t> bytecode;
                bytecode.push_back(static_cast<uint8_t>(VMOpcode::CALL_NATIVE));
                std::string funcName = "MyNativeFunc";
                bytecode.insert(bytecode.end(), funcName.begin(), funcName.end());
                bytecode.push_back('\0'); // Null terminator
                bytecode.push_back(static_cast<uint8_t>(VMOpcode::HALT));
                return bytecode;
            }

            std::queue<std::string> outputQueue;
            std::stack<std::string> operatorStack;
            auto tokens = Tokenize(expression);

            for (const auto& token : tokens) {
                if (isdigit(token[0])) {
                    outputQueue.push(token);
                } else if (token == "(") {
                    operatorStack.push(token);
                } else if (token == ")") {
                    while (!operatorStack.empty() && operatorStack.top() != "(") {
                        outputQueue.push(operatorStack.top());
                        operatorStack.pop();
                    }
                    if (operatorStack.empty()) throw std::runtime_error("Mismatched parentheses");
                    operatorStack.pop(); // Pop the '('
                } else { // Operator
                    while (!operatorStack.empty() && operatorStack.top() != "(" && GetPrecedence(operatorStack.top()[0]) >= GetPrecedence(token[0])) {
                        outputQueue.push(operatorStack.top());
                        operatorStack.pop();
                    }
                    operatorStack.push(token);
                }
            }

            while (!operatorStack.empty()) {
                if (operatorStack.top() == "(") throw std::runtime_error("Mismatched parentheses");
                outputQueue.push(operatorStack.top());
                operatorStack.pop();
            }

            auto bytecode = RpnToBytecode(outputQueue);
            bytecode.push_back(static_cast<uint8_t>(VMOpcode::HALT));
            return bytecode;
        }

    } // namespace VM
} // namespace AetherVisor
