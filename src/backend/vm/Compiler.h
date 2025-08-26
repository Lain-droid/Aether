#pragma once

#include "VMOpcodes.h"
#include <vector>
#include <string>

namespace AetherVisor {
    namespace VM {

        // A simple compiler to translate expressions into our custom bytecode.
        class Compiler {
        public:
            Compiler();

            // Compiles an infix mathematical expression into a bytecode stream.
            std::vector<uint8_t> Compile(const std::string& expression);
        };

    } // namespace VM
} // namespace AetherVisor
