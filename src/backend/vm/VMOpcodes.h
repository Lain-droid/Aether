#pragma once

#include <cstdint>

namespace AetherVisor {
    namespace VM {

        enum class VMOpcode : uint8_t {
            // --- Stack Manipulation ---
            PUSH_INT,       // Pushes a 4-byte integer onto the stack.
            PUSH_FLOAT,     // Pushes a 4-byte float onto the stack.
            PUSH_DOUBLE,    // Pushes an 8-byte double onto the stack.
            PUSH_STR,       // Pushes a null-terminated string onto the stack.
            PUSH_CONST,     // Pushes a constant from constant pool
            POP,            // Pops a value from the stack.
            DUP,            // Duplicates top stack value
            SWAP,           // Swaps top two stack values
            LOAD_LOCAL,     // Loads local variable onto stack
            STORE_LOCAL,    // Stores stack top to local variable
            LOAD_GLOBAL,    // Loads global variable onto stack
            STORE_GLOBAL,   // Stores stack top to global variable

            // --- Arithmetic ---
            ADD,            // Adds the top two stack values, pushes the result.
            SUB,            // Subtracts the top two stack values, pushes the result.
            MUL,            // Multiplies the top two stack values
            DIV,            // Divides the top two stack values
            MOD,            // Modulo operation
            NEG,            // Negates top stack value
            INC,            // Increments top stack value
            DEC,            // Decrements top stack value

            // --- Bitwise Operations ---
            BIT_AND,        // Bitwise AND
            BIT_OR,         // Bitwise OR
            BIT_XOR,        // Bitwise XOR
            BIT_NOT,        // Bitwise NOT
            SHL,            // Shift left
            SHR,            // Shift right

            // --- Logical ---
            AND,            // Logical AND
            OR,             // Logical OR
            NOT,            // Logical NOT

            // --- Comparison ---
            CMP_EQ,         // Pushes 1 if a == b, else 0
            CMP_NE,         // Pushes 1 if a != b, else 0
            CMP_GT,         // Pushes 1 if a > b, else 0
            CMP_GE,         // Pushes 1 if a >= b, else 0
            CMP_LT,         // Pushes 1 if a < b, else 0
            CMP_LE,         // Pushes 1 if a <= b, else 0

            // --- Control Flow ---
            JMP,            // Unconditional jump to a new instruction pointer.
            JMP_IF_ZERO,    // Jumps if the top of the stack is zero.
            JMP_IF_NOT_ZERO,// Jumps if the top of the stack is not zero.
            CALL,           // Function call
            RET,            // Return from function
            RET_VAL,        // Return with value

            // --- Memory Operations ---
            ALLOC,          // Allocate memory
            FREE,           // Free memory
            LOAD_MEM,       // Load from memory address
            STORE_MEM,      // Store to memory address

            // --- Array Operations ---
            ARRAY_NEW,      // Create new array
            ARRAY_GET,      // Get array element
            ARRAY_SET,      // Set array element
            ARRAY_LEN,      // Get array length

            // --- String Operations ---
            STR_CONCAT,     // String concatenation
            STR_LEN,        // String length
            STR_SUBSTR,     // Substring
            STR_CMP,        // String comparison

            // --- Type Operations ---
            CAST_INT,       // Cast to integer
            CAST_FLOAT,     // Cast to float
            CAST_STR,       // Cast to string
            TYPE_OF,        // Get type of value

            // --- Exception Handling ---
            TRY,            // Begin try block
            CATCH,          // Begin catch block
            THROW,          // Throw exception
            FINALLY,        // Finally block

            // --- Advanced Operations ---
            LAMBDA,         // Create lambda function
            CLOSURE,        // Create closure
            EVAL,           // Evaluate string as code
            YIELD,          // Yield value (generators)

            // --- Native Interoperability ---
            CALL_NATIVE,    // Calls a registered native C++ function.
            LOAD_NATIVE,    // Load native library
            GET_NATIVE_FUNC,// Get native function pointer

            // --- Security & Anti-Analysis ---
            ENCRYPT,        // Encrypt stack top
            DECRYPT,        // Decrypt stack top
            HASH,           // Hash stack top
            RAND,           // Generate random number
            OBFUSCATE,      // Obfuscate next instruction
            ANTI_DEBUG,     // Anti-debugging check
            ANTI_VM,        // Anti-VM check

            // --- JIT Operations ---
            JIT_COMPILE,    // Compile to native code
            JIT_EXECUTE,    // Execute native code
            PROFILE,        // Profile code execution

            // --- VM Control ---
            NOP,            // No operation
            HALT,           // Stops execution of the VM.
            PAUSE,          // Pause execution
            RESUME,         // Resume execution
            RESET,          // Reset VM state
            DEBUG_BREAK     // Debug breakpoint
        };

        // Data types supported by the VM
        enum class VMDataType : uint8_t {
            INT32,
            INT64,
            FLOAT32,
            FLOAT64,
            STRING,
            BOOLEAN,
            ARRAY,
            OBJECT,
            FUNCTION,
            NATIVE_PTR,
            ENCRYPTED,
            UNDEFINED
        };

        // VM Value structure
        struct VMValue {
            VMDataType type;
            union {
                int32_t i32;
                int64_t i64;
                float f32;
                double f64;
                bool boolean;
                void* ptr;
                struct {
                    char* data;
                    size_t length;
                } string;
                struct {
                    VMValue* elements;
                    size_t count;
                    size_t capacity;
                } array;
            } data;

            VMValue() : type(VMDataType::UNDEFINED) {}
            VMValue(int32_t val) : type(VMDataType::INT32) { data.i32 = val; }
            VMValue(int64_t val) : type(VMDataType::INT64) { data.i64 = val; }
            VMValue(float val) : type(VMDataType::FLOAT32) { data.f32 = val; }
            VMValue(double val) : type(VMDataType::FLOAT64) { data.f64 = val; }
            VMValue(bool val) : type(VMDataType::BOOLEAN) { data.boolean = val; }
            VMValue(const char* str);
            ~VMValue();
        };

        // Function signature for the VM
        struct VMFunction {
            uint32_t address;           // Start address in bytecode
            uint32_t local_count;       // Number of local variables
            uint32_t param_count;       // Number of parameters
            bool is_native;             // Is this a native function?
            void* native_ptr;           // Pointer to native function
            char name[64];              // Function name
        };

        // Exception handling structure
        struct VMException {
            uint32_t pc;                // Program counter where exception occurred
            VMDataType error_type;      // Type of error
            char message[256];          // Error message
            VMValue error_value;        // Associated error value
        };

        // Represents a single instruction for our VM.
        struct VMInstruction {
            VMOpcode opcode;
            uint32_t operand1;          // First operand
            uint32_t operand2;          // Second operand
            uint32_t operand3;          // Third operand (for complex instructions)
        };

        // Constant pool entry
        struct VMConstant {
            VMDataType type;
            VMValue value;
            bool is_encrypted;          // Security feature
            uint32_t access_count;      // Usage tracking for optimization
        };

        // Security context for VM execution
        struct VMSecurityContext {
            bool allow_native_calls;
            bool allow_memory_alloc;
            bool allow_file_access;
            bool allow_network_access;
            bool enable_anti_debug;
            bool enable_obfuscation;
            uint32_t max_execution_time;
            uint32_t max_memory_usage;
            uint32_t max_stack_depth;
        };

    } // namespace VM
} // namespace AetherVisor
