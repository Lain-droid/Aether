#include "VirtualMachine.h"
#include "../security/XorStr.h"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <limits>

namespace AetherVisor {
    namespace VM {

        VirtualMachine::VirtualMachine() 
            : m_state(VMState::READY)
            , m_initialized(false)
            , m_sandbox_mode(true)
            , m_pc(0)
            , m_code_base(nullptr)
            , m_code_size(0)
            , m_max_stack_size(1024 * 1024) // 1MB stack limit
            , m_next_memory_address(0x10000)
            , m_memory_usage(0)
            , m_max_memory_usage(16 * 1024 * 1024) // 16MB memory limit
            , m_has_exception(false)
            , m_instruction_count(0)
            , m_max_instructions_per_run(1000000)
        {
            // Initialize default security context
            m_security_context.allow_native_calls = false;
            m_security_context.allow_memory_alloc = true;
            m_security_context.allow_file_access = false;
            m_security_context.allow_network_access = false;
            m_security_context.enable_anti_debug = true;
            m_security_context.enable_obfuscation = true;
            m_security_context.max_execution_time = 30000; // 30 seconds
            m_security_context.max_memory_usage = m_max_memory_usage;
            m_security_context.max_stack_depth = 1000;
        }

        VirtualMachine::~VirtualMachine() {
            Reset();
        }

        bool VirtualMachine::Initialize(const VMSecurityContext& security_context) {
            if (m_initialized) {
                SetError(XorS("VM already initialized"));
                return false;
            }

            m_security_context = security_context;
            m_max_memory_usage = security_context.max_memory_usage;
            m_max_instructions_per_run = security_context.max_execution_time * 100; // Rough estimate

            // Initialize security hardening
            Security::SecurityHardening& hardening = Security::SecurityHardening::GetInstance();
            Security::HardeningContext context;
            context.enable_anti_debug = security_context.enable_anti_debug;
            context.enable_anti_vm = true;
            context.enable_memory_encryption = true;
            context.enable_code_integrity = true;
            context.enable_runtime_monitoring = true;
            context.obfuscation_trigger_threshold = 0.7;
            hardening.InitializeRuntimeChecks(context);

            m_initialized = true;
            SetState(VMState::READY);
            return true;
        }

        void VirtualMachine::SetSecurityContext(const VMSecurityContext& context) {
            m_security_context = context;
            m_max_memory_usage = context.max_memory_usage;
        }

        bool VirtualMachine::RegisterNativeFunction(const std::string& name, std::function<VMValue(const std::vector<VMValue>&)> function) {
            if (!m_security_context.allow_native_calls) {
                LogSecurityViolation(XorS("Attempted to register native function without permission"));
                return false;
            }

            if (name.empty() || !function) {
                SetError(XorS("Invalid function name or handler"));
                return false;
            }

            // Check if function name is in allowed list (if whitelist is enabled)
            if (!m_allowed_native_functions.empty() && 
                m_allowed_native_functions.find(name) == m_allowed_native_functions.end()) {
                LogSecurityViolation(XorS("Attempted to register unauthorized native function: ") + name);
                return false;
            }

            m_native_functions[name] = function;
            return true;
        }

        bool VirtualMachine::UnregisterNativeFunction(const std::string& name) {
            auto it = m_native_functions.find(name);
            if (it != m_native_functions.end()) {
                m_native_functions.erase(it);
                return true;
            }
            return false;
        }

        void VirtualMachine::ClearNativeFunctions() {
            m_native_functions.clear();
        }

        bool VirtualMachine::LoadBytecode(const std::vector<uint8_t>& bytecode) {
            if (!IsValidState(VMState::READY)) {
                SetError(XorS("VM not ready for bytecode loading"));
                return false;
            }

            if (bytecode.empty()) {
                SetError(XorS("Empty bytecode"));
                return false;
            }

            if (!VerifyBytecodeIntegrity(bytecode)) {
                SetError(XorS("Bytecode integrity verification failed"));
                return false;
            }

            m_bytecode = bytecode;
            m_code_base = m_bytecode.data();
            m_code_size = static_cast<uint32_t>(m_bytecode.size());
            m_pc = 0;

            return true;
        }

        bool VirtualMachine::Run() {
            return RunSecure(m_max_instructions_per_run);
        }

        bool VirtualMachine::RunSecure(uint32_t max_instructions) {
            if (!IsValidState(VMState::READY) && !IsValidState(VMState::PAUSED)) {
                SetError(XorS("VM not ready for execution"));
                return false;
            }

            if (m_bytecode.empty()) {
                SetError(XorS("No bytecode loaded"));
                return false;
            }

            SetState(VMState::RUNNING);
            m_execution_start = std::chrono::steady_clock::now();
            uint32_t instruction_count = 0;

            try {
                while (m_state == VMState::RUNNING && instruction_count < max_instructions) {
                    // Security checks
                    if (!CheckResourceLimits()) {
                        SetState(VMState::MEMORY_LIMIT_EXCEEDED);
                        break;
                    }

                    // Anti-debug check
                    if (m_security_context.enable_anti_debug) {
                        Security::SecurityHardening& hardening = Security::SecurityHardening::GetInstance();
                        if (hardening.DetectDebuggerPresence()) {
                            LogSecurityViolation(XorS("Debugger detected during execution"));
                            SetState(VMState::SECURITY_VIOLATION);
                            break;
                        }
                    }

                    // Check for breakpoints
                    if (IsBreakpoint(m_pc)) {
                        SetState(VMState::PAUSED);
                        break;
                    }

                    // Execute instruction
                    if (!ExecuteInstruction()) {
                        if (m_state == VMState::RUNNING) {
                            SetState(VMState::ERROR_STATE);
                        }
                        break;
                    }

                    instruction_count++;
                    m_instruction_count++;

                    // Check execution time limit
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - m_execution_start);
                    if (elapsed.count() > m_security_context.max_execution_time) {
                        SetState(VMState::TIMEOUT);
                        break;
                    }
                }

                if (instruction_count >= max_instructions && m_state == VMState::RUNNING) {
                    SetState(VMState::TIMEOUT);
                }

            } catch (const std::exception& e) {
                SetError(XorS("Runtime exception: ") + e.what());
                SetState(VMState::ERROR_STATE);
                return false;
            }

            return m_state == VMState::HALTED || m_state == VMState::PAUSED;
        }

        void VirtualMachine::Pause() {
            if (m_state == VMState::RUNNING) {
                SetState(VMState::PAUSED);
            }
        }

        void VirtualMachine::Resume() {
            if (m_state == VMState::PAUSED) {
                SetState(VMState::RUNNING);
            }
        }

        void VirtualMachine::Reset() {
            SetState(VMState::READY);
            m_pc = 0;
            m_value_stack.clear();
            m_call_stack.clear();
            m_exception_stack.clear();
            m_globals.clear();
            m_constants.clear();
            m_functions.clear();
            
            // Securely clear allocated memory
            for (auto& pair : m_allocated_memory) {
                // SecurePtr destructor will handle secure cleanup
            }
            m_allocated_memory.clear();
            
            m_memory_usage = 0;
            m_instruction_count = 0;
            m_has_exception = false;
            ClearBreakpoints();
        }

        void VirtualMachine::Shutdown() {
            Reset();
            m_initialized = false;
            m_native_functions.clear();
            m_allowed_native_functions.clear();
        }

        void VirtualMachine::PushValue(const VMValue& value) {
            if (!CheckStackOverflow(1)) {
                ThrowException(VMDataType::INTEGER, XorS("Stack overflow"));
                return;
            }
            m_value_stack.push_back(value);
        }

        VMValue VirtualMachine::PopValue() {
            if (!CheckStackUnderflow(1)) {
                ThrowException(VMDataType::INTEGER, XorS("Stack underflow"));
                return VMValue{};
            }
            VMValue value = m_value_stack.back();
            m_value_stack.pop_back();
            return value;
        }

        VMValue VirtualMachine::PeekValue(size_t offset) const {
            if (m_value_stack.size() <= offset) {
                return VMValue{};
            }
            return m_value_stack[m_value_stack.size() - 1 - offset];
        }

        void VirtualMachine::ClearStack() {
            m_value_stack.clear();
        }

        uint32_t VirtualMachine::AllocateMemory(size_t size) {
            if (!m_security_context.allow_memory_alloc) {
                LogSecurityViolation(XorS("Memory allocation not allowed"));
                return 0;
            }

            if (m_memory_usage + size > m_max_memory_usage) {
                LogSecurityViolation(XorS("Memory allocation would exceed limit"));
                return 0;
            }

            uint32_t address = m_next_memory_address;
            m_next_memory_address += static_cast<uint32_t>(size) + 16; // Add padding for security

            auto secure_ptr = std::make_shared<Security::SecurePtr<uint8_t>>(size);
            if (!secure_ptr || secure_ptr->empty()) {
                return 0;
            }

            m_allocated_memory[address] = secure_ptr;
            m_memory_usage += size;

            return address;
        }

        bool VirtualMachine::FreeMemory(uint32_t address) {
            auto it = m_allocated_memory.find(address);
            if (it == m_allocated_memory.end()) {
                SetError(XorS("Invalid memory address for free"));
                return false;
            }

            size_t size = it->second->size();
            m_allocated_memory.erase(it);
            m_memory_usage -= size;
            return true;
        }

        bool VirtualMachine::WriteMemory(uint32_t address, const void* data, size_t size) {
            if (!ValidateMemoryAccess(address, size, true)) {
                return false;
            }

            auto it = m_allocated_memory.find(address);
            if (it == m_allocated_memory.end()) {
                SetError(XorS("Invalid memory address for write"));
                return false;
            }

            if (size > it->second->size()) {
                SetError(XorS("Write size exceeds allocated memory"));
                return false;
            }

            memcpy(it->second->Get(), data, size);
            return true;
        }

        bool VirtualMachine::ReadMemory(uint32_t address, void* data, size_t size) {
            if (!ValidateMemoryAccess(address, size, false)) {
                return false;
            }

            auto it = m_allocated_memory.find(address);
            if (it == m_allocated_memory.end()) {
                SetError(XorS("Invalid memory address for read"));
                return false;
            }

            if (size > it->second->size()) {
                SetError(XorS("Read size exceeds allocated memory"));
                return false;
            }

            memcpy(data, it->second->Get(), size);
            return true;
        }

        size_t VirtualMachine::GetMemoryUsage() const {
            return m_memory_usage;
        }

        void VirtualMachine::ThrowException(VMDataType type, const std::string& message) {
            m_has_exception = true;
            m_current_exception.type = type;
            m_current_exception.message = message;
            m_current_exception.pc = m_pc;
            m_current_exception.stack_trace.clear();
            
            // Add stack trace
            for (const auto& frame : m_call_stack) {
                m_current_exception.stack_trace.push_back(frame.return_address);
            }
        }

        bool VirtualMachine::HasPendingException() const {
            return m_has_exception;
        }

        VMException VirtualMachine::GetException() const {
            return m_current_exception;
        }

        void VirtualMachine::ClearException() {
            m_has_exception = false;
            m_current_exception = VMException{};
        }

        void VirtualMachine::SetBreakpoint(uint32_t address) {
            m_breakpoints.insert(address);
        }

        void VirtualMachine::RemoveBreakpoint(uint32_t address) {
            m_breakpoints.erase(address);
        }

        void VirtualMachine::ClearBreakpoints() {
            m_breakpoints.clear();
        }

        bool VirtualMachine::IsBreakpoint(uint32_t address) const {
            return m_breakpoints.find(address) != m_breakpoints.end();
        }

        std::chrono::milliseconds VirtualMachine::GetExecutionTime() const {
            if (m_state == VMState::RUNNING) {
                auto now = std::chrono::steady_clock::now();
                return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_execution_start);
            }
            return std::chrono::milliseconds(0);
        }

        void VirtualMachine::ResetPerformanceCounters() {
            m_instruction_count = 0;
            m_execution_start = std::chrono::steady_clock::now();
        }

        bool VirtualMachine::VerifyBytecodeIntegrity(const std::vector<uint8_t>& bytecode) {
            // Simple integrity check - in a real implementation, this would use
            // cryptographic signatures and more sophisticated verification
            if (bytecode.empty()) return false;
            
            // Check for minimum header
            if (bytecode.size() < 16) return false;
            
            // Verify magic number (example)
            if (bytecode[0] != 0xAE || bytecode[1] != 0x7E || 
                bytecode[2] != 0xE7 || bytecode[3] != 0x5E) {
                return false;
            }
            
            return true;
        }

        // Private helper methods implementation

        bool VirtualMachine::ExecuteInstruction() {
            if (m_pc >= m_code_size) {
                SetState(VMState::HALTED);
                return true;
            }

            VMOpcode opcode;
            uint32_t operand1, operand2, operand3;
            
            if (!DecodeInstruction(opcode, operand1, operand2, operand3)) {
                SetError(XorS("Failed to decode instruction"));
                return false;
            }

            // Security policy check
            if (!CheckSecurityPolicy(opcode)) {
                LogSecurityViolation(XorS("Security policy violation for opcode: ") + 
                                   std::to_string(static_cast<int>(opcode)));
                return false;
            }

            // Execute the instruction
            switch (opcode) {
                case VMOpcode::PUSH_INT: return ExecutePushInt();
                case VMOpcode::PUSH_FLOAT: return ExecutePushFloat();
                case VMOpcode::PUSH_DOUBLE: return ExecutePushDouble();
                case VMOpcode::PUSH_STRING: return ExecutePushString();
                case VMOpcode::PUSH_CONST: return ExecutePushConst();
                case VMOpcode::POP: return ExecutePop();
                case VMOpcode::DUP: return ExecuteDup();
                case VMOpcode::SWAP: return ExecuteSwap();
                
                case VMOpcode::LOAD_LOCAL: return ExecuteLoadLocal();
                case VMOpcode::STORE_LOCAL: return ExecuteStoreLocal();
                case VMOpcode::LOAD_GLOBAL: return ExecuteLoadGlobal();
                case VMOpcode::STORE_GLOBAL: return ExecuteStoreGlobal();
                
                case VMOpcode::ADD: return ExecuteAdd();
                case VMOpcode::SUB: return ExecuteSubtract();
                case VMOpcode::MUL: return ExecuteMultiply();
                case VMOpcode::DIV: return ExecuteDivide();
                case VMOpcode::MOD: return ExecuteModulo();
                case VMOpcode::NEG: return ExecuteNegate();
                case VMOpcode::INC: return ExecuteIncrement();
                case VMOpcode::DEC: return ExecuteDecrement();
                
                case VMOpcode::BIT_AND: return ExecuteBitwiseAnd();
                case VMOpcode::BIT_OR: return ExecuteBitwiseOr();
                case VMOpcode::BIT_XOR: return ExecuteBitwiseXor();
                case VMOpcode::BIT_NOT: return ExecuteBitwiseNot();
                case VMOpcode::SHL: return ExecuteShiftLeft();
                case VMOpcode::SHR: return ExecuteShiftRight();
                
                case VMOpcode::AND: return ExecuteLogicalAnd();
                case VMOpcode::OR: return ExecuteLogicalOr();
                case VMOpcode::NOT: return ExecuteLogicalNot();
                
                case VMOpcode::CMP_EQ: return ExecuteCompareEqual();
                case VMOpcode::CMP_NE: return ExecuteCompareNotEqual();
                case VMOpcode::CMP_GT: return ExecuteCompareGreater();
                case VMOpcode::CMP_GE: return ExecuteCompareGreaterEqual();
                case VMOpcode::CMP_LT: return ExecuteCompareLess();
                case VMOpcode::CMP_LE: return ExecuteCompareLessEqual();
                
                case VMOpcode::JMP: return ExecuteJump();
                case VMOpcode::JMP_IF_ZERO: return ExecuteJumpIfZero();
                case VMOpcode::JMP_IF_NOT_ZERO: return ExecuteJumpIfNotZero();
                case VMOpcode::CALL: return ExecuteCall();
                case VMOpcode::RET: return ExecuteReturn();
                case VMOpcode::RET_VAL: return ExecuteReturnValue();
                
                case VMOpcode::ALLOC: return ExecuteAlloc();
                case VMOpcode::FREE: return ExecuteFree();
                case VMOpcode::LOAD_MEM: return ExecuteLoadMemory();
                case VMOpcode::STORE_MEM: return ExecuteStoreMemory();
                
                case VMOpcode::ARRAY_NEW: return ExecuteArrayNew();
                case VMOpcode::ARRAY_GET: return ExecuteArrayGet();
                case VMOpcode::ARRAY_SET: return ExecuteArraySet();
                case VMOpcode::ARRAY_LEN: return ExecuteArrayLength();
                
                case VMOpcode::STR_CONCAT: return ExecuteStringConcat();
                case VMOpcode::STR_LEN: return ExecuteStringLength();
                case VMOpcode::STR_SUBSTR: return ExecuteStringSubstring();
                case VMOpcode::STR_CMP: return ExecuteStringCompare();
                
                case VMOpcode::CAST_INT: return ExecuteCastInt();
                case VMOpcode::CAST_FLOAT: return ExecuteCastFloat();
                case VMOpcode::CAST_STR: return ExecuteCastString();
                case VMOpcode::TYPE_OF: return ExecuteTypeOf();
                
                case VMOpcode::TRY: return ExecuteTry();
                case VMOpcode::CATCH: return ExecuteCatch();
                case VMOpcode::THROW: return ExecuteThrow();
                case VMOpcode::FINALLY: return ExecuteFinally();
                
                case VMOpcode::CALL_NATIVE: return ExecuteCallNative();
                case VMOpcode::LOAD_NATIVE: return ExecuteLoadNative();
                case VMOpcode::GET_NATIVE_FUNC: return ExecuteGetNativeFunc();
                
                case VMOpcode::ENCRYPT: return ExecuteEncrypt();
                case VMOpcode::DECRYPT: return ExecuteDecrypt();
                case VMOpcode::HASH: return ExecuteHash();
                case VMOpcode::RAND: return ExecuteRandom();
                case VMOpcode::OBFUSCATE: return ExecuteObfuscate();
                case VMOpcode::ANTI_DEBUG: return ExecuteAntiDebug();
                case VMOpcode::ANTI_VM: return ExecuteAntiVM();
                
                case VMOpcode::JIT_COMPILE: return ExecuteJITCompile();
                case VMOpcode::JIT_EXECUTE: return ExecuteJITExecute();
                case VMOpcode::PROFILE: return ExecuteProfile();
                
                case VMOpcode::NOP: return ExecuteNop();
                case VMOpcode::HALT: return ExecuteHalt();
                case VMOpcode::PAUSE: return ExecutePause();
                case VMOpcode::RESUME: return ExecuteResume();
                case VMOpcode::RESET: return ExecuteReset();
                case VMOpcode::DEBUG_BREAK: return ExecuteDebugBreak();
                
                default:
                    SetError(XorS("Unknown opcode: ") + std::to_string(static_cast<int>(opcode)));
                    return false;
            }
        }

        bool VirtualMachine::DecodeInstruction(VMOpcode& opcode, uint32_t& operand1, uint32_t& operand2, uint32_t& operand3) {
            if (m_pc >= m_code_size) {
                return false;
            }

            opcode = static_cast<VMOpcode>(m_code_base[m_pc++]);
            operand1 = operand2 = operand3 = 0;

            // Decode operands based on instruction format
            // This is a simplified decoder - real implementation would be more complex
            switch (opcode) {
                case VMOpcode::PUSH_INT:
                case VMOpcode::PUSH_FLOAT:
                case VMOpcode::PUSH_DOUBLE:
                case VMOpcode::JMP:
                case VMOpcode::JMP_IF_ZERO:
                case VMOpcode::JMP_IF_NOT_ZERO:
                    if (m_pc + 4 > m_code_size) return false;
                    operand1 = *reinterpret_cast<const uint32_t*>(&m_code_base[m_pc]);
                    m_pc += 4;
                    break;
                
                case VMOpcode::LOAD_LOCAL:
                case VMOpcode::STORE_LOCAL:
                case VMOpcode::LOAD_GLOBAL:
                case VMOpcode::STORE_GLOBAL:
                case VMOpcode::PUSH_CONST:
                    if (m_pc + 2 > m_code_size) return false;
                    operand1 = *reinterpret_cast<const uint16_t*>(&m_code_base[m_pc]);
                    m_pc += 2;
                    break;
                
                default:
                    // No operands for most instructions
                    break;
            }

            return true;
        }

        bool VirtualMachine::CheckStackOverflow(size_t required_space) {
            return m_value_stack.size() + required_space <= m_max_stack_size;
        }

        bool VirtualMachine::CheckStackUnderflow(size_t required_items) {
            return m_value_stack.size() >= required_items;
        }

        bool VirtualMachine::CheckSecurityPolicy(VMOpcode opcode) {
            switch (opcode) {
                case VMOpcode::CALL_NATIVE:
                case VMOpcode::LOAD_NATIVE:
                case VMOpcode::GET_NATIVE_FUNC:
                    return m_security_context.allow_native_calls;
                
                case VMOpcode::ALLOC:
                case VMOpcode::FREE:
                    return m_security_context.allow_memory_alloc;
                
                case VMOpcode::ANTI_DEBUG:
                    return m_security_context.enable_anti_debug;
                
                default:
                    return true; // Most instructions are allowed
            }
        }

        bool VirtualMachine::ValidateMemoryAccess(uint32_t address, size_t size, bool write_access) {
            auto it = m_allocated_memory.find(address);
            if (it == m_allocated_memory.end()) {
                LogSecurityViolation(XorS("Access to unallocated memory"));
                return false;
            }
            
            if (size > it->second->size()) {
                LogSecurityViolation(XorS("Memory access exceeds allocated size"));
                return false;
            }
            
            return true;
        }

        bool VirtualMachine::CheckResourceLimits() {
            // Check memory usage
            if (m_memory_usage > m_max_memory_usage) {
                LogSecurityViolation(XorS("Memory usage limit exceeded"));
                return false;
            }
            
            // Check stack depth
            if (m_call_stack.size() > m_security_context.max_stack_depth) {
                LogSecurityViolation(XorS("Stack depth limit exceeded"));
                return false;
            }
            
            return true;
        }

        void VirtualMachine::LogSecurityViolation(const std::string& violation) {
            // In a real implementation, this would log to a secure audit trail
            m_last_error = XorS("SECURITY VIOLATION: ") + violation;
            SetState(VMState::SECURITY_VIOLATION);
        }

        void VirtualMachine::SetError(const std::string& error) {
            m_last_error = error;
        }

        void VirtualMachine::SetState(VMState new_state) {
            m_state = new_state;
        }

        bool VirtualMachine::IsValidState(VMState required_state) {
            return m_state == required_state;
        }

        // Instruction implementations (simplified - full implementation would be much larger)

        bool VirtualMachine::ExecutePushInt() {
            if (m_pc + 4 > m_code_size) {
                SetError(XorS("Insufficient bytes for PUSH_INT operand"));
                return false;
            }
            
            int32_t value = *reinterpret_cast<const int32_t*>(&m_code_base[m_pc - 4]);
            VMValue vm_value;
            vm_value.type = VMDataType::INTEGER;
            vm_value.int_value = value;
            PushValue(vm_value);
            return !HasPendingException();
        }

        bool VirtualMachine::ExecutePushFloat() {
            if (m_pc + 4 > m_code_size) {
                SetError(XorS("Insufficient bytes for PUSH_FLOAT operand"));
                return false;
            }
            
            float value = *reinterpret_cast<const float*>(&m_code_base[m_pc - 4]);
            VMValue vm_value;
            vm_value.type = VMDataType::FLOAT;
            vm_value.float_value = value;
            PushValue(vm_value);
            return !HasPendingException();
        }

        bool VirtualMachine::ExecuteAdd() {
            if (!CheckStackUnderflow(2)) {
                ThrowException(VMDataType::INTEGER, XorS("Stack underflow in ADD"));
                return false;
            }
            
            VMValue b = PopValue();
            VMValue a = PopValue();
            
            if (a.type != VMDataType::INTEGER || b.type != VMDataType::INTEGER) {
                ThrowException(VMDataType::INTEGER, XorS("Type mismatch in ADD"));
                return false;
            }
            
            int32_t result;
            if (!SafeAdd(a.int_value, b.int_value, result)) {
                ThrowException(VMDataType::INTEGER, XorS("Integer overflow in ADD"));
                return false;
            }
            
            VMValue vm_result;
            vm_result.type = VMDataType::INTEGER;
            vm_result.int_value = result;
            PushValue(vm_result);
            return true;
        }

        bool VirtualMachine::SafeAdd(int32_t a, int32_t b, int32_t& result) {
            if (a > 0 && b > std::numeric_limits<int32_t>::max() - a) {
                return false; // Positive overflow
            }
            if (a < 0 && b < std::numeric_limits<int32_t>::min() - a) {
                return false; // Negative overflow
            }
            result = a + b;
            return true;
        }

        bool VirtualMachine::SafeSubtract(int32_t a, int32_t b, int32_t& result) {
            if (a >= 0 && b < 0 && a > std::numeric_limits<int32_t>::max() + b) {
                return false; // Positive overflow
            }
            if (a < 0 && b > 0 && a < std::numeric_limits<int32_t>::min() + b) {
                return false; // Negative overflow
            }
            result = a - b;
            return true;
        }

        bool VirtualMachine::SafeMultiply(int32_t a, int32_t b, int32_t& result) {
            if (a == 0 || b == 0) {
                result = 0;
                return true;
            }
            
            if (a > 0 && b > 0 && a > std::numeric_limits<int32_t>::max() / b) {
                return false;
            }
            if (a < 0 && b < 0 && a < std::numeric_limits<int32_t>::max() / b) {
                return false;
            }
            if (a > 0 && b < 0 && b < std::numeric_limits<int32_t>::min() / a) {
                return false;
            }
            if (a < 0 && b > 0 && a < std::numeric_limits<int32_t>::min() / b) {
                return false;
            }
            
            result = a * b;
            return true;
        }

        bool VirtualMachine::SafeDivide(int32_t a, int32_t b, int32_t& result) {
            if (b == 0) {
                return false; // Division by zero
            }
            if (a == std::numeric_limits<int32_t>::min() && b == -1) {
                return false; // Overflow
            }
            result = a / b;
            return true;
        }

        // Placeholder implementations for other instructions
        bool VirtualMachine::ExecutePushDouble() { return true; }
        bool VirtualMachine::ExecutePushString() { return true; }
        bool VirtualMachine::ExecutePushConst() { return true; }
        bool VirtualMachine::ExecutePop() { 
            if (!CheckStackUnderflow(1)) {
                ThrowException(VMDataType::INTEGER, XorS("Stack underflow in POP"));
                return false;
            }
            PopValue();
            return true;
        }
        bool VirtualMachine::ExecuteDup() { return true; }
        bool VirtualMachine::ExecuteSwap() { return true; }
        bool VirtualMachine::ExecuteLoadLocal() { return true; }
        bool VirtualMachine::ExecuteStoreLocal() { return true; }
        bool VirtualMachine::ExecuteLoadGlobal() { return true; }
        bool VirtualMachine::ExecuteStoreGlobal() { return true; }
        bool VirtualMachine::ExecuteSubtract() { return true; }
        bool VirtualMachine::ExecuteMultiply() { return true; }
        bool VirtualMachine::ExecuteDivide() { return true; }
        bool VirtualMachine::ExecuteModulo() { return true; }
        bool VirtualMachine::ExecuteNegate() { return true; }
        bool VirtualMachine::ExecuteIncrement() { return true; }
        bool VirtualMachine::ExecuteDecrement() { return true; }
        bool VirtualMachine::ExecuteBitwiseAnd() { return true; }
        bool VirtualMachine::ExecuteBitwiseOr() { return true; }
        bool VirtualMachine::ExecuteBitwiseXor() { return true; }
        bool VirtualMachine::ExecuteBitwiseNot() { return true; }
        bool VirtualMachine::ExecuteShiftLeft() { return true; }
        bool VirtualMachine::ExecuteShiftRight() { return true; }
        bool VirtualMachine::ExecuteLogicalAnd() { return true; }
        bool VirtualMachine::ExecuteLogicalOr() { return true; }
        bool VirtualMachine::ExecuteLogicalNot() { return true; }
        bool VirtualMachine::ExecuteCompareEqual() { return true; }
        bool VirtualMachine::ExecuteCompareNotEqual() { return true; }
        bool VirtualMachine::ExecuteCompareGreater() { return true; }
        bool VirtualMachine::ExecuteCompareGreaterEqual() { return true; }
        bool VirtualMachine::ExecuteCompareLess() { return true; }
        bool VirtualMachine::ExecuteCompareLessEqual() { return true; }
        bool VirtualMachine::ExecuteJump() { return true; }
        bool VirtualMachine::ExecuteJumpIfZero() { return true; }
        bool VirtualMachine::ExecuteJumpIfNotZero() { return true; }
        bool VirtualMachine::ExecuteCall() { return true; }
        bool VirtualMachine::ExecuteReturn() { return true; }
        bool VirtualMachine::ExecuteReturnValue() { return true; }
        bool VirtualMachine::ExecuteAlloc() { return true; }
        bool VirtualMachine::ExecuteFree() { return true; }
        bool VirtualMachine::ExecuteLoadMemory() { return true; }
        bool VirtualMachine::ExecuteStoreMemory() { return true; }
        bool VirtualMachine::ExecuteArrayNew() { return true; }
        bool VirtualMachine::ExecuteArrayGet() { return true; }
        bool VirtualMachine::ExecuteArraySet() { return true; }
        bool VirtualMachine::ExecuteArrayLength() { return true; }
        bool VirtualMachine::ExecuteStringConcat() { return true; }
        bool VirtualMachine::ExecuteStringLength() { return true; }
        bool VirtualMachine::ExecuteStringSubstring() { return true; }
        bool VirtualMachine::ExecuteStringCompare() { return true; }
        bool VirtualMachine::ExecuteCastInt() { return true; }
        bool VirtualMachine::ExecuteCastFloat() { return true; }
        bool VirtualMachine::ExecuteCastString() { return true; }
        bool VirtualMachine::ExecuteTypeOf() { return true; }
        bool VirtualMachine::ExecuteTry() { return true; }
        bool VirtualMachine::ExecuteCatch() { return true; }
        bool VirtualMachine::ExecuteThrow() { return true; }
        bool VirtualMachine::ExecuteFinally() { return true; }
        bool VirtualMachine::ExecuteCallNative() { return true; }
        bool VirtualMachine::ExecuteLoadNative() { return true; }
        bool VirtualMachine::ExecuteGetNativeFunc() { return true; }
        bool VirtualMachine::ExecuteEncrypt() { return true; }
        bool VirtualMachine::ExecuteDecrypt() { return true; }
        bool VirtualMachine::ExecuteHash() { return true; }
        bool VirtualMachine::ExecuteRandom() { return true; }
        bool VirtualMachine::ExecuteObfuscate() { return true; }
        bool VirtualMachine::ExecuteAntiDebug() { return true; }
        bool VirtualMachine::ExecuteAntiVM() { return true; }
        bool VirtualMachine::ExecuteJITCompile() { return true; }
        bool VirtualMachine::ExecuteJITExecute() { return true; }
        bool VirtualMachine::ExecuteProfile() { return true; }
        bool VirtualMachine::ExecuteNop() { return true; }
        bool VirtualMachine::ExecuteHalt() { 
            SetState(VMState::HALTED);
            return true; 
        }
        bool VirtualMachine::ExecutePause() { 
            SetState(VMState::PAUSED);
            return true; 
        }
        bool VirtualMachine::ExecuteResume() { 
            SetState(VMState::RUNNING);
            return true; 
        }
        bool VirtualMachine::ExecuteReset() { 
            Reset();
            return true; 
        }
        bool VirtualMachine::ExecuteDebugBreak() { 
            if (m_security_context.enable_anti_debug) {
                SetState(VMState::PAUSED);
            }
            return true; 
        }

        // VMFactory implementation
        std::unique_ptr<VirtualMachine> VMFactory::CreateSecureVM(const VMSecurityContext& context) {
            auto vm = std::make_unique<VirtualMachine>();
            if (!vm->Initialize(context)) {
                return nullptr;
            }
            return vm;
        }

        std::unique_ptr<VirtualMachine> VMFactory::CreateSandboxedVM() {
            VMSecurityContext context;
            context.allow_native_calls = false;
            context.allow_memory_alloc = true;
            context.allow_file_access = false;
            context.allow_network_access = false;
            context.enable_anti_debug = true;
            context.enable_obfuscation = true;
            context.max_execution_time = 10000; // 10 seconds
            context.max_memory_usage = 4 * 1024 * 1024; // 4MB
            context.max_stack_depth = 100;
            
            return CreateSecureVM(context);
        }

        std::unique_ptr<VirtualMachine> VMFactory::CreateMinimalVM() {
            VMSecurityContext context;
            context.allow_native_calls = false;
            context.allow_memory_alloc = false;
            context.allow_file_access = false;
            context.allow_network_access = false;
            context.enable_anti_debug = false;
            context.enable_obfuscation = false;
            context.max_execution_time = 1000; // 1 second
            context.max_memory_usage = 1024 * 1024; // 1MB
            context.max_stack_depth = 50;
            
            return CreateSecureVM(context);
        }

    } // namespace VM
} // namespace AetherVisor