#pragma once

#include "VMOpcodes.h"
#include <vector>
#include <string>
#include <map>
#include <functional>

namespace AetherVisor {
    namespace VM {

        // A basic stack-based Virtual Machine.
        class VirtualMachine {
        public:
            VirtualMachine();

            // Registers a C++ function so it can be called from within the VM.
            void RegisterNativeFunction(const std::string& name, std::function<void()> function);

            // Executes a given stream of bytecode.
            bool Run(const std::vector<uint8_t>& bytecode);

        private:
            // The VM's stack.
            std::vector<uint8_t> m_stack;

            // The instruction pointer.
            const uint8_t* m_ip;

            // A map of registered native functions.
            std::map<std::string, std::function<void()>> m_nativeFunctions;

            // Stack manipulation helpers
            void Push(int32_t value);
            int32_t Pop();
        };

    } // namespace VM
} // namespace AetherVisor
