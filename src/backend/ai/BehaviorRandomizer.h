#pragma once

#ifdef _WIN32
#include <windows.h>
#else
// Linux/Unix compatibility
typedef struct {
    int x;
    int y;
} POINT;
#endif
#include <vector>
#include <chrono>
#include <random>
#include <unordered_map>
#include <memory>
#include <functional>
#include <mutex>
#include <queue>
#include "SecurityTypes.h"

namespace Aether::AI {

    // Forward declarations
    class AIController;
    class MousePatternGenerator;
    class KeyboardPatternGenerator;
    class TimingAnalyzer;
    class BehaviorLearner;

    /**
     * @brief AI-Based Behavior Randomizer for Human-like Activity Simulation
     * @details Provides sophisticated behavior randomization to evade Hyperion's
     *          AI pattern detection by simulating realistic human interaction patterns
     */
    class BehaviorRandomizer {
    public:
        enum class BehaviorType {
            MouseMovement,
            MouseClicks,
            KeyboardTyping,
            WindowInteraction,
            ApplicationUsage,
            NetworkActivity,
            FileSystemAccess,
            ScriptExecution,
            ProcessActivity
        };

        enum class HumanLikenessLevel {
            Low = 1,        // Basic randomization
            Medium = 2,     // Moderate human-like patterns
            High = 3,       // Advanced human simulation
            Expert = 4,     // Professional human behavior
            AILearned = 5   // AI-learned from real human data
        };

        struct BehaviorPattern {
            BehaviorType type;
            std::vector<double> timingDistribution;
            std::vector<double> intensityDistribution;
            std::vector<std::pair<int, int>> coordinatePatterns;
            double averageInterval;
            double varianceCoefficient;
            double burstiness;
            double predictability;
            std::chrono::steady_clock::time_point lastUpdate;
        };

        struct HumanActivity {
            BehaviorType type;
            std::chrono::steady_clock::time_point timestamp;
            std::vector<double> parameters;
            double humanLikenessScore;
            bool wasDetected;
        };

        struct RandomizationProfile {
            std::string profileName;
            HumanLikenessLevel level;
            std::unordered_map<BehaviorType, BehaviorPattern> patterns;
            std::vector<double> personalityTraits;
            double fatigueLevel;
            double stressLevel;
            std::chrono::steady_clock::time_point creationTime;
            uint64_t usageCount;
        };

        struct DetectionMetrics {
            uint64_t totalActivities;
            uint64_t detectedPatterns;
            uint64_t successfulEvasions;
            double averageHumanLikeness;
            std::unordered_map<BehaviorType, uint32_t> detectionsByType;
            std::chrono::milliseconds totalRandomizationTime;
        };

    private:
        std::unique_ptr<AIController> m_aiController;
        std::unique_ptr<MousePatternGenerator> m_mouseGenerator;
        std::unique_ptr<KeyboardPatternGenerator> m_keyboardGenerator;
        std::unique_ptr<TimingAnalyzer> m_timingAnalyzer;
        std::unique_ptr<BehaviorLearner> m_behaviorLearner;

        RandomizationProfile m_currentProfile;
        std::vector<RandomizationProfile> m_profileDatabase;
        std::queue<HumanActivity> m_activityQueue;
        DetectionMetrics m_metrics;

        mutable std::mutex m_profileMutex;
        mutable std::mutex m_activityMutex;

        std::mt19937_64 m_randomEngine;
        std::normal_distribution<double> m_normalDistribution;
        std::exponential_distribution<double> m_exponentialDistribution;

        // Configuration
        struct RandomizerConfig {
            HumanLikenessLevel defaultLevel = HumanLikenessLevel::High;
            bool enableAILearning = true;
            bool enableRealTimeAdaptation = true;
            bool enableFatigueSimulation = true;
            bool enableStressSimulation = true;
            std::chrono::minutes profileSwitchInterval{30};
            uint32_t maxActivityHistory = 10000;
            double detectionThreshold = 0.8;
        } m_config;

    public:
        explicit BehaviorRandomizer(std::shared_ptr<AIController> aiController);
        ~BehaviorRandomizer();

        // Initialization and lifecycle
        bool Initialize();
        void Shutdown();
        bool LoadProfile(const std::string& profileName);
        bool SaveProfile(const RandomizationProfile& profile);

        // Behavior generation
        std::vector<POINT> GenerateMouseMovement(POINT start, POINT end);
        std::vector<std::chrono::milliseconds> GenerateClickTiming(uint32_t clickCount);
        std::vector<std::chrono::milliseconds> GenerateTypingRhythm(const std::string& text);
        std::chrono::milliseconds GenerateExecutionDelay();

        // Pattern randomization
        void RandomizeMousePattern();
        void RandomizeKeyboardPattern();
        void RandomizeTimingPattern();
        void RandomizeWindowInteraction();

        // Human simulation
        void SimulateHumanBehavior(BehaviorType type, std::chrono::milliseconds duration);
        void SimulateFatigue(double fatigueLevel);
        void SimulateStress(double stressLevel);
        void SimulatePersonalityTraits(const std::vector<double>& traits);

        // AI-guided adaptation
        void AdaptToDetectionAttempt(BehaviorType detectedType);
        void UpdateBehaviorModel();
        RandomizationProfile GenerateAIOptimizedProfile();

        // Real-time behavior modification
        void ModifyBehaviorInRealTime(BehaviorType type, double intensity);
        void InsertRandomInterruptions();
        void AddNaturalPauses();

        // Detection evasion
        bool DetectPatternAnalysis();
        void ActivateAntiPatternMeasures();
        void ScrambleBehaviorSignature();

        // Profile management
        std::vector<RandomizationProfile> GetAvailableProfiles();
        RandomizationProfile GetCurrentProfile() const;
        bool SwitchProfile(const std::string& profileName);
        void AutoSwitchProfile();

        // Metrics and analysis
        DetectionMetrics GetMetrics() const { return m_metrics; }
        double CalculateHumanLikeness(const std::vector<HumanActivity>& activities);
        void AnalyzeBehaviorEffectiveness();
        void ExportBehaviorLog(const std::string& filePath);

        // Configuration
        void SetHumanLikenessLevel(HumanLikenessLevel level);
        void EnableFeature(const std::string& feature, bool enable);
        void SetDetectionThreshold(double threshold);

    private:
        // Core randomization algorithms
        double GenerateHumanTiming(double baseInterval, double variance);
        std::vector<double> GenerateBurstPattern(size_t count, double intensity);
        std::vector<POINT> GenerateNaturalMousePath(POINT start, POINT end);

        // AI learning and adaptation
        void LearnFromDetection(BehaviorType type, const std::vector<double>& parameters);
        void UpdatePersonalityModel();
        void OptimizeBehaviorParameters();

        // Human characteristics simulation
        void ApplyFatigueEffect(BehaviorPattern& pattern);
        void ApplyStressEffect(BehaviorPattern& pattern);
        void ApplyPersonalityEffect(BehaviorPattern& pattern, const std::vector<double>& traits);

        // Pattern analysis and generation
        BehaviorPattern AnalyzeHumanData(BehaviorType type);
        std::vector<double> ExtractTimingFeatures(const std::vector<std::chrono::milliseconds>& timings);
        double CalculatePatternComplexity(const BehaviorPattern& pattern);

        // Utility methods
        double ApplyJitter(double value, double jitterFactor);
        std::vector<double> SmoothTimingData(const std::vector<double>& rawTimings);
        bool IsPatternDetectable(const BehaviorPattern& pattern);
    };

    /**
     * @brief Mouse Pattern Generator for realistic mouse movements and clicks
     */
    class MousePatternGenerator {
    public:
        enum class MovementStyle {
            Direct,         // Straight line movement
            Natural,        // Natural curved movement
            Hesitant,       // Uncertain movement with pauses
            Confident,      // Smooth confident movement
            Tired,          // Slower, less precise movement
            Stressed        // Erratic movement patterns
        };

        struct MouseProfile {
            MovementStyle style;
            double speed;           // Base movement speed
            double acceleration;    // Acceleration factor
            double jitter;          // Mouse jitter amount
            double overshoot;       // Overshoot tendency
            double correctionTime;  // Time to correct overshoot
        };

    private:
        MouseProfile m_currentProfile;
        std::mt19937 m_randomEngine;

    public:
        bool Initialize();
        std::vector<POINT> GenerateMovement(POINT start, POINT end, MovementStyle style);
        std::vector<std::chrono::microseconds> GenerateClickTimings(uint32_t clickCount);
        void SetMouseProfile(const MouseProfile& profile);
        MouseProfile GenerateRandomProfile();
    };

    /**
     * @brief Keyboard Pattern Generator for realistic typing patterns
     */
    class KeyboardPatternGenerator {
    public:
        enum class TypingStyle {
            Hunt_Peck,      // Two-finger typing
            Touch_Typing,   // Professional touch typing
            Fast_Typing,    // Fast but error-prone
            Slow_Typing,    // Careful slow typing
            Programming,    // Programming-specific patterns
            Gaming          // Gaming-optimized patterns
        };

        struct TypingProfile {
            TypingStyle style;
            double averageWPM;      // Words per minute
            double errorRate;       // Error frequency
            double rhythmVariation; // Timing variation
            std::unordered_map<char, double> keyDelays; // Per-key delays
        };

    private:
        TypingProfile m_currentProfile;
        std::unordered_map<std::string, double> m_digramTimings; // Two-character combinations

    public:
        bool Initialize();
        std::vector<std::chrono::milliseconds> GenerateTypingRhythm(const std::string& text);
        void SetTypingProfile(const TypingProfile& profile);
        TypingProfile GenerateRandomProfile();
        void LearnFromTypingData(const std::vector<std::chrono::milliseconds>& actualTimings);
    };

    /**
     * @brief Timing Analyzer for temporal pattern analysis
     */
    class TimingAnalyzer {
    public:
        struct TimingStatistics {
            double mean;
            double variance;
            double skewness;
            double kurtosis;
            double entropy;
            std::vector<double> autocorrelation;
        };

    private:
        std::vector<std::chrono::milliseconds> m_timingHistory;
        size_t m_maxHistorySize;

    public:
        bool Initialize(size_t maxHistorySize = 10000);
        void AddTiming(std::chrono::milliseconds timing);
        TimingStatistics AnalyzeTimings();
        double CalculateHumanLikeness(const TimingStatistics& stats);
        bool DetectArtificialPattern();
    };

    /**
     * @brief Behavior Learner for AI-based behavior optimization
     */
    class BehaviorLearner {
    public:
        struct LearningData {
            BehaviorRandomizer::BehaviorType type;
            std::vector<double> features;
            bool wasDetected;
            double humanLikenessScore;
            std::chrono::steady_clock::time_point timestamp;
        };

    private:
        std::vector<LearningData> m_trainingData;
        std::shared_ptr<AIController> m_aiController;

    public:
        explicit BehaviorLearner(std::shared_ptr<AIController> aiController);
        bool Initialize();
        void AddTrainingData(const LearningData& data);
        BehaviorRandomizer::BehaviorPattern OptimizePattern(BehaviorRandomizer::BehaviorType type);
        std::vector<double> PredictOptimalParameters(BehaviorRandomizer::BehaviorType type);
        void UpdateModel();
    };

} // namespace Aether::AI