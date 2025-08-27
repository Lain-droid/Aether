#pragma once

#include "VMOpcodes.h"
#include "../security/XorStr.h"
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace AetherVisor {
    namespace VM {

        // JIT compilation result
        struct JITCompilationResult {
            bool success;
            std::vector<uint8_t> native_code;
            size_t code_size;
            void* executable_memory;
            std::string error_message;
            
            // Performance metadata
            size_t original_bytecode_size;
            double compilation_time_ms;
            uint32_t optimization_level;
        };

        // JIT function entry point
        typedef int(*JITFunction)(void* vm_context, void* args);

        // JIT compiler settings
        struct JITSettings {
            bool enable_optimizations;
            bool enable_security_checks;
            bool enable_profiling;
            uint32_t optimization_level; // 0-3
            size_t max_code_cache_size;
            bool enable_code_encryption;
        };

        // Advanced JIT Compiler for VM bytecode to native code
        class JITCompiler {
        public:
            JITCompiler();
            ~JITCompiler();

            // Initialize JIT compiler with settings
            bool Initialize(const JITSettings& settings);
            
            // Compile bytecode to native code
            JITCompilationResult Compile(const std::vector<uint8_t>& bytecode, 
                                       const std::string& function_name = "");
            
            // Execute JIT compiled function
            int Execute(const JITCompilationResult& compiled_code, void* vm_context, void* args);
            
            // Code cache management
            bool CacheCompiledCode(const std::string& key, const JITCompilationResult& result);
            JITCompilationResult* GetCachedCode(const std::string& key);
            void ClearCodeCache();
            size_t GetCacheSizeBytes() const;
            
            // Memory management
            void* AllocateExecutableMemory(size_t size);
            void FreeExecutableMemory(void* memory, size_t size);
            
            // Optimization passes
            std::vector<uint8_t> OptimizeBasicBlocks(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> OptimizeConstantPropagation(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> OptimizeDeadCodeElimination(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> OptimizeInstructionCombining(const std::vector<uint8_t>& bytecode);
            
            // Security features
            void EnableCodeEncryption(bool enable) { m_settings.enable_code_encryption = enable; }
            bool VerifyCodeIntegrity(const JITCompilationResult& result);
            void ApplyAntiTampering(JITCompilationResult& result);
            
            // Profiling and debugging
            void EnableProfiling(bool enable) { m_settings.enable_profiling = enable; }
            std::map<std::string, double> GetProfilingData() const;
            void ResetProfilingData();

        private:
            JITSettings m_settings;
            std::map<std::string, std::unique_ptr<JITCompilationResult>> m_code_cache;
            size_t m_cache_size_bytes;
            bool m_initialized;
            
            // Code generation
            struct CodeGenerator {
                std::vector<uint8_t> machine_code;
                std::map<uint32_t, uint32_t> label_map; // bytecode address -> machine code offset
                std::vector<std::pair<uint32_t, uint32_t>> relocations; // (offset, target)
                
                void EmitByte(uint8_t byte);
                void EmitWord(uint16_t word);
                void EmitDWord(uint32_t dword);
                void EmitInstruction(const std::vector<uint8_t>& instruction);
                void EmitLabel(uint32_t label_id);
                void EmitJump(uint32_t target_label);
                void EmitCall(uint32_t target_label);
                void ApplyRelocations();
            };
            
            // x86-64 instruction encoding helpers
            void EmitPrologue(CodeGenerator& gen);
            void EmitEpilogue(CodeGenerator& gen);
            void EmitStackOperation(CodeGenerator& gen, VMOpcode opcode, uint32_t operand);
            void EmitArithmetic(CodeGenerator& gen, VMOpcode opcode);
            void EmitComparison(CodeGenerator& gen, VMOpcode opcode);
            void EmitControlFlow(CodeGenerator& gen, VMOpcode opcode, uint32_t operand);
            void EmitMemoryOperation(CodeGenerator& gen, VMOpcode opcode, uint32_t operand);
            void EmitSecurityCheck(CodeGenerator& gen, VMOpcode opcode);
            
            // Instruction translation
            bool TranslateInstruction(CodeGenerator& gen, VMOpcode opcode, 
                                    uint32_t operand1, uint32_t operand2, uint32_t operand3);
            
            // Optimization analysis
            struct BasicBlock {
                uint32_t start_address;
                uint32_t end_address;
                std::vector<VMOpcode> instructions;
                std::vector<uint32_t> predecessors;
                std::vector<uint32_t> successors;
                bool is_loop_header;
                uint32_t execution_count; // For profiling
            };
            
            std::vector<BasicBlock> AnalyzeControlFlow(const std::vector<uint8_t>& bytecode);
            void OptimizeBasicBlock(BasicBlock& block);
            
            // Security and anti-tampering
            uint32_t CalculateChecksum(const std::vector<uint8_t>& code);
            void EncryptCode(std::vector<uint8_t>& code);
            void DecryptCode(std::vector<uint8_t>& code);
            void InsertAntiTamperingChecks(CodeGenerator& gen);
            
            // Platform-specific code
#ifdef _WIN32
            void* AllocateExecutableMemoryWindows(size_t size);
            void FreeExecutableMemoryWindows(void* memory, size_t size);
#else
            void* AllocateExecutableMemoryPosix(size_t size);
            void FreeExecutableMemoryPosix(void* memory, size_t size);
#endif
            
            // Error handling
            void SetError(JITCompilationResult& result, const std::string& error);
            
            // Profiling data
            mutable std::map<std::string, double> m_profiling_data;
        };

        // JIT function registry for managing compiled functions
        class JITFunctionRegistry {
        public:
            static JITFunctionRegistry& GetInstance();
            
            bool RegisterFunction(const std::string& name, JITFunction function, void* metadata = nullptr);
            JITFunction GetFunction(const std::string& name);
            bool UnregisterFunction(const std::string& name);
            void ClearRegistry();
            
            std::vector<std::string> GetRegisteredFunctions() const;
            
        private:
            struct FunctionEntry {
                JITFunction function;
                void* metadata;
                std::chrono::time_point<std::chrono::steady_clock> registration_time;
            };
            
            std::map<std::string, FunctionEntry> m_functions;
        };

    } // namespace VM
} // namespace AetherVisor