#pragma once

#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <chrono>
#include "AIController.h"

namespace AetherVisor {
    namespace Payload {

        /**
         * @brief AI-Synchronized Hook Manager with Intelligence and Adaptive Behavior
         * @details Enhanced hook system that synchronizes with AI for optimal evasion
         */
        class Hooks {
        public:
            // Type definition for the callback that forwards console output
            using ConsoleOutputCallback = std::function<void(const std::string& output)>;
            
            enum class HookStrategy {
                STEALTH_MODE,      // Minimal detection footprint
                ADAPTIVE_MODE,     // AI-guided hook placement
                AGGRESSIVE_MODE,   // Maximum functionality
                EVASION_MODE      // Full evasion protocols
            };

            struct HookMetrics {
                uint64_t totalHookCalls = 0;
                uint64_t suspiciousDetections = 0;
                uint64_t adaptiveAdjustments = 0;
                std::chrono::milliseconds totalHookTime{0};
                double evasionSuccessRate = 1.0;
            };

            // AI-enhanced installation with intelligent placement
            static bool InstallWithAI(ConsoleOutputCallback callback, 
                                    std::shared_ptr<Backend::AIController> aiController);
            
            // Adaptive hook management
            static void SetHookStrategy(HookStrategy strategy);
            static void AdaptToDetection();
            static void OptimizeHookPlacement();
            
            // AI synchronization methods
            static void SyncWithAI();
            static void UpdateAIThreatAssessment();
            static bool ShouldMaintainHooks();
            
            // Traditional methods (now AI-enhanced)
            static bool Install(ConsoleOutputCallback callback);
            static void Uninstall();
            
            // Monitoring and metrics
            static HookMetrics GetMetrics() { return m_metrics; }
            static void ResetMetrics();
            static bool IsHookCompromised();

        private:
            static void* m_originalPrintFunc;
            static void* m_originalWarnFunc;
            static std::shared_ptr<Backend::AIController> m_aiController;
            static HookStrategy m_currentStrategy;
            static HookMetrics m_metrics;
            static std::mutex m_hookMutex;
            static std::chrono::steady_clock::time_point m_lastOptimization;
            
            // AI-enhanced hook handlers
            static void AI_HookedPrintFunction(const std::string& output);
            static void AI_HookedWarnFunction(const std::string& output);
            
            // Intelligence methods
            static void AnalyzeHookUsage();
            static void UpdateHookBehavior();
            static bool DetectHookAnalysis();
            static void ActivateCountermeasures();
        };

    } // namespace Payload
} // namespace AetherVisor
