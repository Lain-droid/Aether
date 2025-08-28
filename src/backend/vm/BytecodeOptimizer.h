#pragma once

#include "VMOpcodes.h"
#include "../security/XorStr.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <functional>

namespace AetherVisor {
    namespace VM {

        // Optimization level settings
        enum class OptimizationLevel {
            NONE = 0,
            BASIC = 1,      // Dead code elimination, constant folding
            MEDIUM = 2,     // Basic + jump optimization, peephole
            AGGRESSIVE = 3  // Medium + advanced analysis, inlining
        };

        // Optimization statistics
        struct OptimizationStats {
            size_t original_size;
            size_t optimized_size;
            size_t instructions_removed;
            size_t instructions_combined;
            size_t constants_folded;
            size_t jumps_optimized;
            double optimization_time_ms;
            std::vector<std::string> applied_optimizations;
        };

        // Control flow graph node
        struct CFGNode {
            uint32_t start_address;
            uint32_t end_address;
            std::vector<uint8_t> instructions;
            std::set<uint32_t> predecessors;
            std::set<uint32_t> successors;
            bool is_entry_point;
            bool is_exit_point;
            bool is_loop_header;
            uint32_t execution_frequency; // For profile-guided optimization
        };

        // Advanced bytecode optimizer with sophisticated analysis
        class BytecodeOptimizer {
        public:
            BytecodeOptimizer();
            ~BytecodeOptimizer() = default;

            // Main optimization interface
            std::vector<uint8_t> Optimize(const std::vector<uint8_t>& bytecode, 
                                        OptimizationLevel level = OptimizationLevel::MEDIUM);
            
            // Specific optimization passes
            std::vector<uint8_t> EliminateDeadCode(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> FoldConstants(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> OptimizeJumps(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> PeepholeOptimization(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> InlineSmallFunctions(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> OptimizeStackOperations(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> EliminateRedundantLoads(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> OptimizeLoops(const std::vector<uint8_t>& bytecode);
            
            // Control flow analysis
            std::vector<CFGNode> BuildControlFlowGraph(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> OptimizeControlFlow(const std::vector<CFGNode>& cfg);
            
            // Data flow analysis
            std::map<uint32_t, std::set<uint32_t>> AnalyzeDataFlow(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> OptimizeDataFlow(const std::vector<uint8_t>& bytecode, 
                                                const std::map<uint32_t, std::set<uint32_t>>& data_flow);
            
            // Advanced optimizations
            std::vector<uint8_t> VectorizeOperations(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> UnrollLoops(const std::vector<uint8_t>& bytecode, uint32_t max_unroll_factor = 4);
            std::vector<uint8_t> OptimizeMemoryAccess(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> InlineConstantPropagation(const std::vector<uint8_t>& bytecode);
            
            // Security-aware optimizations
            std::vector<uint8_t> ObfuscateControlFlow(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> InsertAntiAnalysisCode(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> EncryptConstants(const std::vector<uint8_t>& bytecode);
            
            // Statistics and analysis
            OptimizationStats GetLastOptimizationStats() const { return m_last_stats; }
            void EnableProfiling(bool enable) { m_profiling_enabled = enable; }
            void SetProfilingData(const std::map<uint32_t, uint32_t>& execution_counts);
            
            // Validation
            bool ValidateBytecode(const std::vector<uint8_t>& bytecode);
            bool VerifyOptimizationCorrectness(const std::vector<uint8_t>& original, 
                                             const std::vector<uint8_t>& optimized);

        private:
            OptimizationStats m_last_stats;
            bool m_profiling_enabled;
            std::map<uint32_t, uint32_t> m_execution_counts;
            
            // Instruction analysis helpers
            struct InstructionInfo {
                VMOpcode opcode;
                std::vector<uint32_t> operands;
                uint32_t address;
                uint32_t size;
                bool is_jump;
                bool is_conditional_jump;
                bool modifies_stack;
                bool reads_memory;
                bool writes_memory;
                std::set<uint32_t> jump_targets;
            };
            
            std::vector<InstructionInfo> AnalyzeInstructions(const std::vector<uint8_t>& bytecode);
            InstructionInfo DecodeInstruction(const uint8_t* code, uint32_t address, uint32_t max_size);
            
            // Pattern matching for optimizations
            struct OptimizationPattern {
                std::vector<VMOpcode> pattern;
                std::vector<VMOpcode> replacement;
                std::function<bool(const std::vector<InstructionInfo>&, size_t)> condition;
                std::string description;
            };
            
            std::vector<OptimizationPattern> m_peephole_patterns;
            void InitializeOptimizationPatterns();
            bool MatchPattern(const std::vector<InstructionInfo>& instructions, 
                            size_t start_index, const OptimizationPattern& pattern);
            
            // Dead code analysis
            std::set<uint32_t> FindReachableInstructions(const std::vector<InstructionInfo>& instructions);
            std::set<uint32_t> FindLiveVariables(const std::vector<InstructionInfo>& instructions, 
                                               uint32_t instruction_index);
            
            // Constant folding helpers
            struct ConstantValue {
                VMDataType type;
                union {
                    int32_t int_val;
                    float float_val;
                    double double_val;
                };
                bool is_known;
            };
            
            std::map<uint32_t, ConstantValue> AnalyzeConstants(const std::vector<InstructionInfo>& instructions);
            bool CanFoldConstantOperation(VMOpcode opcode, const ConstantValue& a, const ConstantValue& b);
            ConstantValue FoldConstantOperation(VMOpcode opcode, const ConstantValue& a, const ConstantValue& b);
            
            // Jump optimization helpers
            std::map<uint32_t, uint32_t> FindJumpTargets(const std::vector<InstructionInfo>& instructions);
            std::set<uint32_t> FindUnreachableJumps(const std::vector<InstructionInfo>& instructions);
            uint32_t ResolveJumpChain(const std::map<uint32_t, uint32_t>& jump_map, uint32_t target);
            
            // Loop analysis
            struct LoopInfo {
                uint32_t header_address;
                uint32_t end_address;
                std::set<uint32_t> body_addresses;
                std::set<uint32_t> exit_addresses;
                bool is_inner_loop;
                uint32_t estimated_iterations;
            };
            
            std::vector<LoopInfo> DetectLoops(const std::vector<CFGNode>& cfg);
            bool IsLoopInvariant(const InstructionInfo& instruction, const LoopInfo& loop);
            
            // Stack simulation for optimization
            struct StackState {
                std::vector<ConstantValue> stack;
                bool is_valid;
            };
            
            StackState SimulateStackOperation(const StackState& current_state, const InstructionInfo& instruction);
            std::map<uint32_t, StackState> AnalyzeStackStates(const std::vector<InstructionInfo>& instructions);
            
            // Code generation helpers
            void EmitInstruction(std::vector<uint8_t>& output, VMOpcode opcode);
            void EmitInstruction(std::vector<uint8_t>& output, VMOpcode opcode, uint32_t operand);
            void EmitInstruction(std::vector<uint8_t>& output, VMOpcode opcode, uint32_t op1, uint32_t op2);
            void EmitInstructionSequence(std::vector<uint8_t>& output, const std::vector<InstructionInfo>& instructions);
            
            // Address translation for jump fixup
            std::map<uint32_t, uint32_t> m_address_translation;
            void UpdateJumpTargets(std::vector<uint8_t>& bytecode, const std::map<uint32_t, uint32_t>& translation);
            
            // Security and anti-analysis
            std::vector<uint8_t> InsertJunkInstructions(const std::vector<uint8_t>& bytecode, double density = 0.1);
            std::vector<uint8_t> SplitBasicBlocks(const std::vector<uint8_t>& bytecode);
            std::vector<uint8_t> InsertFakeJumps(const std::vector<uint8_t>& bytecode);
            
            // Validation helpers
            bool ValidateStackBalance(const std::vector<uint8_t>& bytecode);
            bool ValidateJumpTargets(const std::vector<uint8_t>& bytecode);
            bool ValidateInstructionSequence(const std::vector<uint8_t>& bytecode);
            
            // Metrics and statistics
            void UpdateStatistics(const std::string& optimization_name, 
                                size_t original_size, size_t optimized_size);
            void RecordOptimizationApplication(const std::string& optimization_name);
        };

        // Bytecode analysis utilities
        class BytecodeAnalyzer {
        public:
            static std::vector<uint32_t> FindFunctionBoundaries(const std::vector<uint8_t>& bytecode);
            static std::map<uint32_t, std::string> ExtractStringConstants(const std::vector<uint8_t>& bytecode);
            static std::set<uint32_t> FindCriticalPaths(const std::vector<uint8_t>& bytecode);
            static double EstimateExecutionComplexity(const std::vector<uint8_t>& bytecode);
            static std::vector<uint32_t> FindHotSpots(const std::vector<uint8_t>& bytecode, 
                                                    const std::map<uint32_t, uint32_t>& execution_counts);
        };

    } // namespace VM
} // namespace AetherVisor