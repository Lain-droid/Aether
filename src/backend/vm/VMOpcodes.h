#pragma once

#include <cstdint>

namespace AetherVisor {
    namespace VM {

        enum class VMOpcode : uint8_t {
            // --- Stack Manipulation ---
            PUSH_INT,       // Pushes a 4-byte integer onto the stack.
            PUSH_STR,       // Pushes a null-terminated string onto the stack.
            POP,            // Pops a value from the stack.

            // --- Arithmetic ---
            ADD,            // Adds the top two stack values, pushes the result.
            SUB,            // Subtracts the top two stack values, pushes the result.

            // --- Control Flow ---
            JMP,            // Unconditional jump to a new instruction pointer.
            JMP_IF_ZERO,    // Jumps if the top of the stack is zero.

            // --- Native Interoperability ---
            // This is the most critical opcode for practical use.
            CALL_NATIVE,    // Calls a registered native C++ function.

            // --- VM Control ---
            HALT            // Stops execution of the VM.
        };

        // Represents a single instruction for our VM.
        // In a real bytecode stream, the arguments would follow the opcode byte.
        // This struct is more for conceptual clarity.
        struct VMInstruction {
            VMOpcode opcode;
            // Arguments would be serialized directly into the bytecode stream
            // after the opcode. For example, PUSH_INT would be followed by 4 bytes
            // representing the integer.
            int64_t argument;
        };

    } // namespace VM
} // namespace AetherVisor
