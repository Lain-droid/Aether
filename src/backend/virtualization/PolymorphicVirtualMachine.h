#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <random>
#include <functional>
#include <chrono>
#include <mutex>
#include "SecurityTypes.h"

namespace Aether::Virtualization {

    // Forward declarations
    class AIController;
    class OpcodeMutator;
    class PatternlessInterpreter;
    class PolymorphicEngine;

    /**
     * @brief Polymorphic Virtual Machine with dynamic opcode transformation
     * @details Provides a virtualization layer that changes its bytecode representation
     *          on every execution to prevent signature detection by Hyperion
     */
    class PolymorphicVirtualMachine {
    public:
        enum class VirtualOpcode : uint32_t {
            // Basic operations (dynamically mapped)
            VM_NOP = 0x00,
            VM_LOAD_CONST = 0x01,
            VM_LOAD_VAR = 0x02,
            VM_STORE_VAR = 0x03,
            VM_ADD = 0x04,
            VM_SUB = 0x05,
            VM_MUL = 0x06,
            VM_DIV = 0x07,
            VM_MOD = 0x08,
            VM_AND = 0x09,
            VM_OR = 0x0A,
            VM_XOR = 0x0B,
            VM_NOT = 0x0C,
            VM_SHL = 0x0D,
            VM_SHR = 0x0E,
            VM_CMP = 0x0F,
            VM_JMP = 0x10,
            VM_JE = 0x11,
            VM_JNE = 0x12,
            VM_JL = 0x13,
            VM_JG = 0x14,
            VM_CALL = 0x15,
            VM_RET = 0x16,
            VM_PUSH = 0x17,
            VM_POP = 0x18,
            VM_HALT = 0x19,
            
            // Luau-specific operations
            VM_LUA_CALL = 0x20,
            VM_LUA_GETGLOBAL = 0x21,
            VM_LUA_SETGLOBAL = 0x22,
            VM_LUA_GETTABLE = 0x23,
            VM_LUA_SETTABLE = 0x24,
            VM_LUA_CLOSURE = 0x25,
            
            // Obfuscation operations (decoy)
            VM_DECOY_OP1 = 0x30,
            VM_DECOY_OP2 = 0x31,
            VM_DECOY_OP3 = 0x32,
            
            // Special operations
            VM_METAMORPH = 0xFF  // Triggers runtime metamorphosis
        };

        struct VirtualInstruction {
            VirtualOpcode opcode;
            std::vector<uint32_t> operands;
            uint32_t metamorphicKey;  // Used for runtime transformation
            std::vector<uint8_t> padding;  // Random padding for size variation
        };

        struct VirtualMachineState {
            std::vector<uint32_t> stack;
            std::unordered_map<std::string, uint32_t> variables;
            std::vector<VirtualInstruction> instructions;
            size_t programCounter;
            std::unordered_map<VirtualOpcode, uint32_t> opcodeMapping;
            uint64_t sessionSeed;
            std::chrono::steady_clock::time_point lastMetamorph;
        };

        struct MetamorphicProfile {
            std::vector<VirtualOpcode> opcodePermutation;
            std::unordered_map<VirtualOpcode, std::vector<VirtualOpcode>> equivalentOpcodes;
            std::vector<std::function<void()>> decoyOperations;
            uint32_t complexityLevel;
            std::chrono::milliseconds morphInterval;
        };

    private:
        std::unique_ptr<AI::AIController> m_aiController;
        std::unique_ptr<OpcodeMutator> m_opcodeMutator;
        std::unique_ptr<PatternlessInterpreter> m_interpreter;
        std::unique_ptr<PolymorphicEngine> m_polymorphicEngine;

        VirtualMachineState m_vmState;
        MetamorphicProfile m_metamorphicProfile;

        mutable std::mutex m_stateMutex;
        mutable std::mutex m_metamorphMutex;

        std::mt19937_64 m_randomEngine;
        std::uniform_int_distribution<uint32_t> m_opcodeDistribution;
        std::uniform_int_distribution<uint8_t> m_paddingDistribution;

        // Configuration
        struct VMConfig {
            bool enablePolymorphism = true;
            bool enableDecoyOpcodes = true;
            bool enablePaddingRandomization = true;
            std::chrono::milliseconds morphInterval{1000};
            uint32_t maxDecoyOpcodes = 50;
            uint32_t maxPaddingSize = 16;
            double metamorphTriggerProbability = 0.1;
        } m_config;

        // Metrics
        struct VMMetrics {
            uint64_t instructionsExecuted;
            uint64_t metamorphosisCount;
            uint64_t signatureDetectionAttempts;
            uint64_t successfulEvasions;
            std::chrono::milliseconds totalExecutionTime;
            std::chrono::steady_clock::time_point lastMetrics;
        } m_metrics;

    public:
        explicit PolymorphicVirtualMachine(std::shared_ptr<AI::AIController> aiController);
        ~PolymorphicVirtualMachine();

        // Initialization and lifecycle
        bool Initialize();
        void Shutdown();
        void Reset();

        // Bytecode compilation and loading
        std::vector<VirtualInstruction> CompileLuauScript(const std::string& script);
        bool LoadBytecode(const std::vector<VirtualInstruction>& bytecode);
        std::vector<uint8_t> SerializeBytecode(const std::vector<VirtualInstruction>& bytecode);
        std::vector<VirtualInstruction> DeserializeBytecode(const std::vector<uint8_t>& data);

        // Execution
        bool Execute();
        bool ExecuteInstruction(const VirtualInstruction& instruction);
        bool Step();  // Single step execution
        void Halt();

        // Metamorphic operations
        void TriggerMetamorphosis();
        void RandomizeOpcodeMapping();
        void GenerateDecoyInstructions();
        void ApplyPolymorphicTransformation();

        // AI-guided adaptation
        void AdaptToDetectionAttempt();
        void UpdateMetamorphicProfile();
        std::vector<VirtualOpcode> GenerateAIGuidedPermutation();

        // Security features
        bool DetectSignatureScanning();
        void ActivateAntiAnalysis();
        void InsertTimingVariations();

        // State management
        VirtualMachineState GetState() const;
        bool RestoreState(const VirtualMachineState& state);
        std::vector<uint8_t> SerializeState();
        bool DeserializeState(const std::vector<uint8_t>& data);

        // Metrics and monitoring
        VMMetrics GetMetrics() const { return m_metrics; }
        void ResetMetrics();
        void ExportExecutionTrace(const std::string& filePath);

        // Configuration
        void SetMorphInterval(std::chrono::milliseconds interval) { 
            m_config.morphInterval = interval; 
        }
        void EnablePolymorphism(bool enable) { m_config.enablePolymorphism = enable; }
        void SetComplexityLevel(uint32_t level);

    private:
        // Core execution methods
        bool ExecuteArithmetic(VirtualOpcode opcode, const std::vector<uint32_t>& operands);
        bool ExecuteControl(VirtualOpcode opcode, const std::vector<uint32_t>& operands);
        bool ExecuteLuauOperation(VirtualOpcode opcode, const std::vector<uint32_t>& operands);
        bool ExecuteDecoyOperation(VirtualOpcode opcode);

        // Metamorphic transformations
        void PermuteOpcodes();
        void InsertGarbageInstructions();
        void ReorderInstructions();
        void ApplyEquivalentTransforms();

        // Pattern obfuscation
        void ScrambleInstructionLayout();
        void VarieInstructionSizes();
        void ApplyControlFlowObfuscation();

        // Anti-analysis techniques
        void DetectDebuggerPresence();
        void CheckExecutionTiming();
        void ValidateInstructionIntegrity();

        // Utility methods
        uint32_t GenerateRandomSeed();
        std::vector<uint8_t> GenerateRandomPadding(size_t maxSize);
        VirtualOpcode MapRealOpcode(VirtualOpcode originalOpcode);
        bool IsDecoyOpcode(VirtualOpcode opcode);
    };

    /**
     * @brief Opcode Mutator for dynamic instruction transformation
     */
    class OpcodeMutator {
    public:
        enum class MutationType {
            Substitution,    // Replace with equivalent operation
            Expansion,       // Expand to multiple operations
            Reordering,      // Change operation order
            Padding,         // Add meaningless operations
            Encoding         // Change encoding format
        };

        struct MutationRule {
            VirtualOpcode sourceOpcode;
            MutationType mutationType;
            std::vector<VirtualOpcode> targetOpcodes;
            double mutationProbability;
            uint32_t complexityScore;
        };

    private:
        std::vector<MutationRule> m_mutationRules;
        std::mt19937 m_randomEngine;

    public:
        bool Initialize();
        std::vector<PolymorphicVirtualMachine::VirtualInstruction> MutateInstructions(
            const std::vector<PolymorphicVirtualMachine::VirtualInstruction>& original);
        void AddMutationRule(const MutationRule& rule);
        std::vector<MutationRule> GenerateAIMutationRules();
    };

    /**
     * @brief Patternless Interpreter to prevent signature recognition
     */
    class PatternlessInterpreter {
    public:
        struct ExecutionContext {
            std::unordered_map<PolymorphicVirtualMachine::VirtualOpcode, 
                             std::function<bool(const std::vector<uint32_t>&)>> opcodeHandlers;
            std::vector<uint32_t> executionStack;
            std::unordered_map<std::string, uint32_t> environment;
            uint64_t instructionCounter;
        };

    private:
        ExecutionContext m_context;
        std::vector<std::function<bool(const std::vector<uint32_t>&)>> m_handlerPool;

    public:
        bool Initialize(std::shared_ptr<PolymorphicVirtualMachine> vm);
        bool ExecuteInstruction(PolymorphicVirtualMachine::VirtualOpcode opcode,
                              const std::vector<uint32_t>& operands);
        void RandomizeHandlerAssignment();
        void GenerateHandlerVariations();
    };

    /**
     * @brief Polymorphic Engine for code transformation
     */
    class PolymorphicEngine {
    public:
        enum class TransformationType {
            EquivalentSubstitution,
            DeadCodeInsertion,
            ControlFlowObfuscation,
            RegisterReallocation,
            InstructionReordering
        };

        struct TransformationPass {
            TransformationType type;
            double applicationProbability;
            uint32_t iterations;
            std::function<void(std::vector<PolymorphicVirtualMachine::VirtualInstruction>&)> transform;
        };

    private:
        std::vector<TransformationPass> m_transformationPasses;
        std::shared_ptr<AI::AIController> m_aiController;

    public:
        explicit PolymorphicEngine(std::shared_ptr<AI::AIController> aiController);
        
        bool Initialize();
        std::vector<PolymorphicVirtualMachine::VirtualInstruction> Transform(
            const std::vector<PolymorphicVirtualMachine::VirtualInstruction>& original);
        void AddTransformationPass(const TransformationPass& pass);
        void OptimizeTransformations();
    };

} // namespace Aether::Virtualization