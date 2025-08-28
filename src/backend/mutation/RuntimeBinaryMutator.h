#pragma once

#include <windows.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <mutex>
#include <atomic>
#include <functional>
#include <thread>
#include "SecurityTypes.h"

namespace Aether::Mutation {

    // Forward declarations
    class AI::AIController;
    class CodeAnalyzer;
    class MutationEngine;
    class SignatureEvader;
    class PolymorphicTransformer;

    /**
     * @brief Runtime Binary Mutator for Dynamic Code Morphing
     * @details Provides sophisticated runtime binary mutation capabilities to defeat
     *          signature scanning by continuously morphing executable code while maintaining functionality
     */
    class RuntimeBinaryMutator {
    public:
        enum class MutationType {
            InstructionSubstitution,    // Replace instructions with equivalent ones
            RegisterReallocation,       // Change register usage patterns
            CodeReordering,            // Reorder independent instructions
            NopInsertion,              // Insert harmless NOP instructions
            JumpObfuscation,           // Obfuscate control flow with jumps
            CallTransformation,        // Transform function calls
            DataObfuscation,           // Obfuscate embedded data
            SizeVariation,             // Vary instruction sizes
            PolymorphicEncoding,       // Use different encodings for same operation
            AIGuided                   // AI-selected mutation strategy
        };

        enum class MutationTrigger {
            TimeInterval,              // Mutate at regular intervals
            InjectionCount,            // Mutate after N injections
            ThreatDetection,           // Mutate when threats detected
            SignatureScanning,         // Mutate when scanning detected
            MemoryAccess,              // Mutate on suspicious memory access
            AIAdaptive                 // AI determines when to mutate
        };

        enum class CodeSection {
            EntryPoint,                // Main entry point
            InjectionCode,             // Code injection routines
            CryptoFunctions,           // Cryptographic functions
            AntiDebug,                 // Anti-debugging code
            PayloadLoader,             // Payload loading code
            IPCCode,                   // IPC communication code
            AIComponents,              // AI-related code
            UtilityFunctions          // Utility and helper functions
        };

        struct MutationProfile {
            std::string profileName;
            std::vector<MutationType> enabledMutations;
            std::unordered_map<MutationType, double> mutationProbabilities;
            std::unordered_map<CodeSection, uint32_t> mutationIntensity;
            std::chrono::milliseconds mutationInterval;
            MutationTrigger trigger;
            bool enableAIGuidance;
            uint32_t maxMutationsPerCycle;
        };

        struct CodeRegion {
            LPVOID baseAddress;
            SIZE_T size;
            CodeSection section;
            std::vector<uint8_t> originalCode;
            std::vector<uint8_t> currentCode;
            std::vector<uint8_t> mutationHistory;
            uint32_t mutationCount;
            std::chrono::steady_clock::time_point lastMutation;
            bool isActive;
            bool isProtected;
        };

        struct MutationMetrics {
            uint64_t totalMutations;
            uint64_t successfulMutations;
            uint64_t failedMutations;
            uint64_t signatureEvasions;
            uint64_t detectionAttempts;
            std::unordered_map<MutationType, uint32_t> mutationTypeUsage;
            std::chrono::milliseconds totalMutationTime;
            std::chrono::steady_clock::time_point lastActivity;
            double averageSuccessRate;
        };

    private:
        std::unique_ptr<AI::AIController> m_aiController;
        std::unique_ptr<CodeAnalyzer> m_codeAnalyzer;
        std::unique_ptr<MutationEngine> m_mutationEngine;
        std::unique_ptr<SignatureEvader> m_signatureEvader;
        std::unique_ptr<PolymorphicTransformer> m_polymorphicTransformer;

        MutationProfile m_currentProfile;
        std::vector<CodeRegion> m_codeRegions;
        MutationMetrics m_metrics;

        mutable std::mutex m_regionsMutex;
        mutable std::mutex m_mutationMutex;

        std::atomic<bool> m_isMutationActive;
        std::thread m_mutationThread;
        std::thread m_detectionThread;

        // Mutation state
        std::atomic<uint32_t> m_injectionCount;
        std::chrono::steady_clock::time_point m_lastMutation;
        std::vector<uint8_t> m_currentSignature;

        // Configuration
        struct MutatorConfig {
            bool enableRuntimeMutation = true;
            bool enableSignatureTracking = true;
            bool enableDetectionResponse = true;
            std::chrono::milliseconds defaultMutationInterval{5000};
            uint32_t maxCodeRegions = 100;
            uint32_t mutationRetryLimit = 3;
            double mutationSuccessThreshold = 0.8;
        } m_config;

    public:
        explicit RuntimeBinaryMutator(std::shared_ptr<AI::AIController> aiController);
        ~RuntimeBinaryMutator();

        // Initialization and lifecycle
        bool Initialize();
        void Shutdown();
        bool IsActive() const { return m_isMutationActive.load(); }

        // Code region management
        bool RegisterCodeRegion(LPVOID baseAddress, SIZE_T size, CodeSection section);
        bool UnregisterCodeRegion(LPVOID baseAddress);
        bool ProtectCodeRegion(LPVOID baseAddress, bool protect);
        std::vector<CodeRegion> GetRegisteredRegions() const;

        // Mutation profile management
        bool LoadMutationProfile(const MutationProfile& profile);
        MutationProfile CreateAIOptimizedProfile();
        bool SaveMutationProfile(const std::string& filePath);
        bool LoadMutationProfile(const std::string& filePath);

        // Runtime mutation operations
        bool StartMutation();
        bool StopMutation();
        bool TriggerImmediateMutation();
        bool MutateCodeRegion(LPVOID baseAddress);
        bool MutateAllRegions();

        // Mutation techniques
        bool ApplyInstructionSubstitution(CodeRegion& region);
        bool ApplyRegisterReallocation(CodeRegion& region);
        bool ApplyCodeReordering(CodeRegion& region);
        bool ApplyNopInsertion(CodeRegion& region);
        bool ApplyJumpObfuscation(CodeRegion& region);

        // Detection and evasion
        bool DetectSignatureScanning();
        bool DetectStaticAnalysis();
        bool DetectDynamicAnalysis();
        void ActivateEvasiveMutation();
        bool GenerateNewSignature();

        // AI integration
        MutationType SelectOptimalMutation(const CodeRegion& region);
        bool AdaptToDetectionPattern();
        std::vector<uint8_t> GenerateAIMorphedCode(const std::vector<uint8_t>& originalCode);
        void UpdateAIModel();

        // Validation and integrity
        bool ValidateCodeIntegrity();
        bool TestMutatedCode(const CodeRegion& region);
        bool RollbackMutation(LPVOID baseAddress);
        bool RestoreOriginalCode(LPVOID baseAddress);

        // Metrics and monitoring
        MutationMetrics GetMetrics() const { return m_metrics; }
        void ResetMetrics();
        double CalculateMutationEffectiveness();
        void ExportMutationLog(const std::string& filePath);

        // Configuration
        void SetMutationInterval(std::chrono::milliseconds interval);
        void EnableMutationType(MutationType type, bool enable);
        void SetMutationIntensity(CodeSection section, uint32_t intensity);

        // Event handlers
        void OnInjectionEvent();
        void OnThreatDetected();
        void OnSignatureScanDetected();

    private:
        // Core mutation implementations
        bool MutateCodeInternal(CodeRegion& region);
        std::vector<uint8_t> ApplyMutation(const std::vector<uint8_t>& code, MutationType type);
        bool InstallMutatedCode(CodeRegion& region, const std::vector<uint8_t>& mutatedCode);

        // Code analysis
        bool AnalyzeCodeRegion(CodeRegion& region);
        std::vector<uint32_t> FindMutableInstructions(const std::vector<uint8_t>& code);
        bool IsInstructionMutable(const uint8_t* instruction, size_t maxLength);
        size_t GetInstructionLength(const uint8_t* instruction);

        // Mutation algorithms
        std::vector<uint8_t> SubstituteInstruction(const uint8_t* instruction, size_t length);
        std::vector<uint8_t> ReallocateRegisters(const std::vector<uint8_t>& code);
        std::vector<uint8_t> ReorderInstructions(const std::vector<uint8_t>& code);
        std::vector<uint8_t> InsertNops(const std::vector<uint8_t>& code, uint32_t count);

        // Detection implementations
        bool MonitorMemoryAccess();
        bool CheckSignatureChanges();
        bool AnalyzeAccessPatterns();
        bool DetectEmulationEnvironment();

        // Threading and synchronization
        void MutationLoop();
        void DetectionLoop();
        bool ShouldTriggerMutation();
        void ProcessMutationQueue();

        // Utility methods
        bool BackupCodeRegion(CodeRegion& region);
        bool RestoreCodeRegion(CodeRegion& region);
        std::vector<uint8_t> CalculateCodeSignature(const std::vector<uint8_t>& code);
        bool IsCodeExecutable(LPVOID address, SIZE_T size);
        DWORD ChangeMemoryProtection(LPVOID address, SIZE_T size, DWORD newProtection);
    };

    /**
     * @brief Code Analyzer for static and dynamic code analysis
     */
    class CodeAnalyzer {
    public:
        enum class InstructionType {
            Data,
            Jump,
            Call,
            Arithmetic,
            Logic,
            Memory,
            Control,
            Unknown
        };

        struct InstructionInfo {
            uint32_t offset;
            InstructionType type;
            size_t length;
            std::vector<uint8_t> bytes;
            std::vector<uint32_t> operands;
            bool isMutable;
            bool isControlFlow;
        };

        struct AnalysisResult {
            std::vector<InstructionInfo> instructions;
            std::vector<uint32_t> entryPoints;
            std::vector<uint32_t> jumpTargets;
            std::vector<uint32_t> mutableOffsets;
            uint32_t complexityScore;
        };

    private:
        std::shared_ptr<AI::AIController> m_aiController;

    public:
        explicit CodeAnalyzer(std::shared_ptr<AI::AIController> aiController);
        bool Initialize();
        AnalysisResult AnalyzeCode(const std::vector<uint8_t>& code);
        InstructionInfo AnalyzeInstruction(const uint8_t* instruction, uint32_t offset);
        std::vector<uint32_t> FindMutableInstructions(const AnalysisResult& analysis);
        bool ValidateControlFlow(const AnalysisResult& analysis);
    };

    /**
     * @brief Mutation Engine for code transformation
     */
    class MutationEngine {
    public:
        using MutationFunction = std::function<std::vector<uint8_t>(const std::vector<uint8_t>&)>;

        struct MutationRule {
            RuntimeBinaryMutator::MutationType type;
            std::vector<uint8_t> pattern;
            std::vector<uint8_t> replacement;
            double probability;
            uint32_t priority;
        };

    private:
        std::vector<MutationRule> m_mutationRules;
        std::unordered_map<RuntimeBinaryMutator::MutationType, MutationFunction> m_mutationFunctions;
        std::shared_ptr<AI::AIController> m_aiController;

    public:
        explicit MutationEngine(std::shared_ptr<AI::AIController> aiController);
        bool Initialize();
        std::vector<uint8_t> ApplyMutation(const std::vector<uint8_t>& code, 
                                         RuntimeBinaryMutator::MutationType type);
        void AddMutationRule(const MutationRule& rule);
        std::vector<MutationRule> GenerateAIMutationRules();
        bool ValidateMutation(const std::vector<uint8_t>& original, 
                            const std::vector<uint8_t>& mutated);
    };

    /**
     * @brief Signature Evader for anti-signature techniques
     */
    class SignatureEvader {
    public:
        enum class SignatureType {
            BytePattern,
            HashBased,
            StructuralHash,
            BehavioralSignature,
            MLClassifier
        };

        struct SignatureInfo {
            SignatureType type;
            std::vector<uint8_t> signature;
            double confidence;
            std::chrono::steady_clock::time_point detectionTime;
        };

    private:
        std::vector<SignatureInfo> m_knownSignatures;
        std::shared_ptr<AI::AIController> m_aiController;

    public:
        explicit SignatureEvader(std::shared_ptr<AI::AIController> aiController);
        bool Initialize();
        bool DetectSignature(const std::vector<uint8_t>& code);
        std::vector<uint8_t> EvadeSignature(const std::vector<uint8_t>& code, 
                                          const SignatureInfo& signature);
        void AddKnownSignature(const SignatureInfo& signature);
        std::vector<SignatureInfo> AnalyzeForSignatures(const std::vector<uint8_t>& code);
    };

    /**
     * @brief Polymorphic Transformer for advanced code morphing
     */
    class PolymorphicTransformer {
    public:
        enum class TransformationType {
            Substitution,
            Expansion,
            Compression,
            Permutation,
            Obfuscation
        };

        struct TransformationRule {
            TransformationType type;
            std::vector<uint8_t> inputPattern;
            std::vector<std::vector<uint8_t>> outputVariants;
            uint32_t complexity;
        };

    private:
        std::vector<TransformationRule> m_transformationRules;
        std::shared_ptr<AI::AIController> m_aiController;

    public:
        explicit PolymorphicTransformer(std::shared_ptr<AI::AIController> aiController);
        bool Initialize();
        std::vector<uint8_t> Transform(const std::vector<uint8_t>& code, 
                                     TransformationType type);
        void GenerateTransformationVariants(const std::vector<uint8_t>& code);
        std::vector<uint8_t> CreatePolymorphicVariant(const std::vector<uint8_t>& code);
    };

} // namespace Aether::Mutation