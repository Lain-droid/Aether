#pragma once

#include "VMOpcodes.h"
#include "../security/SecurityHardening.h"
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <stack>
#include <chrono>
#include <memory>
#include <cstdint>
#include <set>

// Forward declarations to avoid circular dependencies
namespace AetherVisor {
    namespace VM {
        enum class VMDataType : uint8_t;
        struct VMValue;
        struct VMFunction;
        struct VMConstant;
        struct VMSecurityContext;
        enum class VMOpcode : uint8_t;
    }
    namespace Security {
        template<typename T> class SecurePtr;
    }
}

namespace AetherVisor {
    namespace VM {

        // VM execution state
        enum class VMState {
            READY,
            RUNNING,
            PAUSED,
            HALTED,
            ERROR_STATE,
            TIMEOUT,
            MEMORY_LIMIT_EXCEEDED,
            STACK_OVERFLOW,
            SECURITY_VIOLATION
        };

        // Call frame for function calls
        struct CallFrame {
            uint32_t return_address;
            uint32_t local_base;
            uint32_t local_count;
            VMFunction* function;
        };

        // Exception handling frame
        struct ExceptionFrame {
            uint32_t handler_address;
            uint32_t stack_size;
            VMDataType exception_type;
        };

        // Advanced secure stack-based Virtual Machine
        class VirtualMachine {
        public:
            VirtualMachine();
            ~VirtualMachine();

            // Security and configuration
            bool Initialize(const VMSecurityContext& security_context);
            void SetSecurityContext(const VMSecurityContext& context);
            const VMSecurityContext& GetSecurityContext() const { return m_security_context; }

            // Native function registration with security checks
            bool RegisterNativeFunction(const std::string& name, std::function<VMValue(const std::vector<VMValue>&)> function);
            bool UnregisterNativeFunction(const std::string& name);
            void ClearNativeFunctions();

            // Bytecode execution with full security
            bool LoadBytecode(const std::vector<uint8_t>& bytecode);
            bool Run();
            bool RunSecure(uint32_t max_instructions = 1000000);
            void Pause();
            void Resume();
            void Reset();
            void Shutdown();

            // State management
            VMState GetState() const { return m_state; }
            bool IsRunning() const { return m_state == VMState::RUNNING; }
            bool HasError() const { return m_state == VMState::ERROR_STATE; }
            std::string GetLastError() const { return m_last_error; }

            // Stack operations with bounds checking
            void PushValue(const VMValue& value);
            VMValue PopValue();
            VMValue PeekValue(size_t offset = 0) const;
            size_t GetStackSize() const { return m_value_stack.size(); }
            void ClearStack();

            // Memory management
            uint32_t AllocateMemory(size_t size);
            bool FreeMemory(uint32_t address);
            bool WriteMemory(uint32_t address, const void* data, size_t size);
            bool ReadMemory(uint32_t address, void* data, size_t size);
            size_t GetMemoryUsage() const { return m_memory_usage; }

            // Function calls
            bool CallFunction(const std::string& name, const std::vector<VMValue>& args, VMValue& result);
            bool CallNativeFunction(const std::string& name, const std::vector<VMValue>& args, VMValue& result);

            // Exception handling
            void ThrowException(VMDataType type, const std::string& message);
            bool HasPendingException() const { return m_has_exception; }
            void ClearException();

            // Debugging and profiling
            void SetBreakpoint(uint32_t address);
            void RemoveBreakpoint(uint32_t address);
            void ClearBreakpoints();
            bool IsBreakpoint(uint32_t address) const;
            
            // Performance monitoring
            uint64_t GetInstructionCount() const { return m_instruction_count; }
            std::chrono::milliseconds GetExecutionTime() const;
            void ResetPerformanceCounters();

            // Security features
            bool VerifyBytecodeIntegrity(const std::vector<uint8_t>& bytecode);
            void EnableSandboxMode(bool enable) { m_sandbox_mode = enable; }
            bool IsSandboxMode() const { return m_sandbox_mode; }

        private:
            // VM state
            VMState m_state;
            VMSecurityContext m_security_context;
            std::string m_last_error;
            bool m_initialized;
            bool m_sandbox_mode;

            // Bytecode and execution
            std::vector<uint8_t> m_bytecode;
            uint32_t m_pc; // Program counter
            const uint8_t* m_code_base;
            uint32_t m_code_size;

            // Stack management
            std::vector<VMValue> m_value_stack;
            std::vector<CallFrame> m_call_stack;
            std::vector<ExceptionFrame> m_exception_stack;
            size_t m_max_stack_size;

            // Memory management with security
            std::map<uint32_t, std::shared_ptr<Security::SecurePtr<uint8_t>>> m_allocated_memory;
            uint32_t m_next_memory_address;
            size_t m_memory_usage;
            size_t m_max_memory_usage;

            // Native functions with enhanced security
            std::map<std::string, std::function<VMValue(const std::vector<VMValue>&)>> m_native_functions;
            std::set<std::string> m_allowed_native_functions;

            // Constants and globals
            std::vector<VMConstant> m_constants;
            std::vector<VMValue> m_globals;
            std::vector<VMFunction> m_functions;

            // Exception handling
            bool m_has_exception;

            // Security and monitoring
            std::set<uint32_t> m_breakpoints;
            uint64_t m_instruction_count;
            std::chrono::time_point<std::chrono::steady_clock> m_execution_start;
            uint32_t m_max_instructions_per_run;

            // Execution helpers
            bool ExecuteInstruction();
            bool DecodeInstruction(VMOpcode& opcode, uint32_t& operand1, uint32_t& operand2, uint32_t& operand3);
            
            // Stack operations (internal)
            bool CheckStackOverflow(size_t required_space);
            bool CheckStackUnderflow(size_t required_items);
            void PushInt32(int32_t value);
            void PushInt64(int64_t value);
            void PushFloat32(float value);
            void PushFloat64(double value);
            void PushString(const std::string& value);
            void PushBoolean(bool value);
            
            int32_t PopInt32();
            int64_t PopInt64();
            float PopFloat32();
            double PopFloat64();
            std::string PopString();
            bool PopBoolean();

            // Arithmetic operations with overflow checking
            bool SafeAdd(int32_t a, int32_t b, int32_t& result);
            bool SafeSubtract(int32_t a, int32_t b, int32_t& result);
            bool SafeMultiply(int32_t a, int32_t b, int32_t& result);
            bool SafeDivide(int32_t a, int32_t b, int32_t& result);

            // Memory operations
            bool IsValidMemoryAddress(uint32_t address, size_t size);
            uint32_t AllocateSecureMemory(size_t size);

            // Function management
            VMFunction* FindFunction(const std::string& name);
            bool ValidateFunctionCall(const VMFunction* func, const std::vector<VMValue>& args);

            // Security checks
            bool CheckSecurityPolicy(VMOpcode opcode);
            bool ValidateMemoryAccess(uint32_t address, size_t size, bool write_access);
            bool CheckResourceLimits();
            void LogSecurityViolation(const std::string& violation);
            
            // Error handling
            void SetError(const std::string& error);
            void SetState(VMState new_state);
            bool IsValidState(VMState required_state);
        };

    } // namespace VM
} // namespace AetherVisor
