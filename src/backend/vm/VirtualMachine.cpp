#include "VirtualMachine.h"
#include <iostream> // For debug logging

namespace AetherVisor {
    namespace VM {

        VirtualMachine::VirtualMachine() {
            m_stack.reserve(1024); // Pre-allocate some stack space
        }

        void VirtualMachine::RegisterNativeFunction(const std::string& name, std::function<void()> function) {
            m_nativeFunctions[name] = function;
        }

        bool VirtualMachine::Run(const std::vector<uint8_t>& bytecode) {
            m_ip = bytecode.data();
            m_stack.clear();

            while (true) {
                VMOpcode opcode = static_cast<VMOpcode>(*m_ip++);

                switch (opcode) {
                    case VMOpcode::HALT: {
                        return true; // Success
                    }
                    case VMOpcode::PUSH_INT: {
                        int32_t value = *(reinterpret_cast<const int32_t*>(m_ip));
                        m_ip += sizeof(int32_t);
                        Push(value);
                        break;
                    }
                    case VMOpcode::POP: {
                        Pop();
                        break;
                    }
                    case VMOpcode::ADD: {
                        int32_t b = Pop();
                        int32_t a = Pop();
                        Push(a + b);
                        break;
                    }
                    case VMOpcode::SUB: {
                        int32_t b = Pop();
                        int32_t a = Pop();
                        Push(a - b);
                        break;
                    }
                    case VMOpcode::MUL: {
                        int32_t b = Pop();
                        int32_t a = Pop();
                        Push(a * b);
                        break;
                    }
                    case VMOpcode::DIV: {
                        int32_t b = Pop();
                        int32_t a = Pop();
                        if (b == 0) return false; // Division by zero
                        Push(a / b);
                        break;
                    }
                    case VMOpcode::AND: {
                        int32_t b = Pop();
                        int32_t a = Pop();
                        Push(a & b);
                        break;
                    }
                    case VMOpcode::OR: {
                        int32_t b = Pop();
                        int32_t a = Pop();
                        Push(a | b);
                        break;
                    }
                    case VMOpcode::NOT: {
                        int32_t a = Pop();
                        Push(~a);
                        break;
                    }
                    case VMOpcode::CMP_EQ: {
                        int32_t b = Pop();
                        int32_t a = Pop();
                        Push((a == b) ? 1 : 0);
                        break;
                    }
                    case VMOpcode::CMP_GT: {
                        int32_t b = Pop();
                        int32_t a = Pop();
                        Push((a > b) ? 1 : 0);
                        break;
                    }
                    case VMOpcode::CALL_NATIVE: {
                        // For simplicity, this assumes the name is pushed to the stack
                        // A better implementation would read it from bytecode.
                        // This part is highly conceptual.
                        std::string funcName = "example_func"; // Placeholder
                        if (m_nativeFunctions.count(funcName)) {
                            m_nativeFunctions[funcName]();
                        }
                        break;
                    }
                    default:
                        // Unknown opcode
                        return false;
                }
            }
            return false; // Should be unreachable
        }

        void VirtualMachine::Push(int32_t value) {
            size_t current_size = m_stack.size();
            m_stack.resize(current_size + sizeof(int32_t));
            *(reinterpret_cast<int32_t*>(&m_stack[current_size])) = value;
        }

        int32_t VirtualMachine::Pop() {
            if (m_stack.size() < sizeof(int32_t)) {
                // Stack underflow
                return 0;
            }
            size_t new_size = m_stack.size() - sizeof(int32_t);
            int32_t value = *(reinterpret_cast<int32_t*>(&m_stack[new_size]));
            m_stack.resize(new_size);
            return value;
        }

    } // namespace VM
} // namespace AetherVisor
