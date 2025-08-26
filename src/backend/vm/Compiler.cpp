#include "Compiler.h"
#include <sstream>
#include <stack>
#include <map>

namespace AetherVisor {
    namespace VM {

        Compiler::Compiler() {}

        // This is a placeholder implementation. A real compiler would involve
        // a full shunting-yard or other parsing algorithm.
        std::vector<uint8_t> Compiler::Compile(const std::string& expression) {
            // This demonstrates the bytecode that WOULD be generated for "5 + 10"
            std::vector<uint8_t> bytecode;

            // 1. PUSH_INT 5
            bytecode.push_back(static_cast<uint8_t>(VMOpcode::PUSH_INT));
            int32_t val1 = 5;
            const uint8_t* val1_bytes = reinterpret_cast<const uint8_t*>(&val1);
            bytecode.insert(bytecode.end(), val1_bytes, val1_bytes + sizeof(int32_t));

            // 2. PUSH_INT 10
            bytecode.push_back(static_cast<uint8_t>(VMOpcode::PUSH_INT));
            int32_t val2 = 10;
            const uint8_t* val2_bytes = reinterpret_cast<const uint8_t*>(&val2);
            bytecode.insert(bytecode.end(), val2_bytes, val2_bytes + sizeof(int32_t));

            // 3. ADD
            bytecode.push_back(static_cast<uint8_t>(VMOpcode::ADD));

            // 4. HALT
            bytecode.push_back(static_cast<uint8_t>(VMOpcode::HALT));

            return bytecode;
        }

    } // namespace VM
} // namespace AetherVisor
