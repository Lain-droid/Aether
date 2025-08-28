#pragma once

#include <chrono>
#include <memory>
#include <atomic>
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <thread>
#include <array>

namespace AetherVisor {
    namespace Performance {

        /**
         * @brief High-Performance Monitoring System
         * @details Zero-overhead performance monitoring with:
         *          - CPU usage tracking
         *          - Memory allocation monitoring  
         *          - I/O performance metrics
         *          - AI system performance
         *          - Real-time bottleneck detection
         */
        class PerformanceMonitor {
        public:
            struct Metrics {
                // CPU metrics
                double cpuUsagePercent = 0.0;
                uint64_t instructionCount = 0;
                uint64_t cacheHits = 0;
                uint64_t cacheMisses = 0;
                
                // Memory metrics
                size_t memoryUsageBytes = 0;
                size_t peakMemoryUsage = 0;
                uint32_t allocationCount = 0;
                uint32_t deallocationCount = 0;
                
                // AI system metrics
                double aiProcessingTime = 0.0;
                uint32_t aiDecisionCount = 0;
                double aiAccuracy = 0.0;
                
                // I/O metrics
                uint64_t bytesRead = 0;
                uint64_t bytesWritten = 0;
                double ioLatency = 0.0;
                
                // Performance scores
                double overallPerformanceScore = 0.0;
                double securityOverhead = 0.0;
            };

            struct ProfilePoint {
                std::string name;
                std::chrono::high_resolution_clock::time_point startTime;
                std::chrono::high_resolution_clock::time_point endTime;
                uint64_t instructionsBefore = 0;
                uint64_t instructionsAfter = 0;
                size_t memoryBefore = 0;
                size_t memoryAfter = 0;
            };

            // Singleton access
            static PerformanceMonitor& GetInstance();

            // Core functions
            bool Initialize();
            void Shutdown();
            
            // Profiling
            void StartProfiling(const std::string& name);
            void EndProfiling(const std::string& name);
            void RecordProfilePoint(const ProfilePoint& point);
            
            // Metrics collection
            void UpdateMetrics();
            Metrics GetCurrentMetrics() const;
            Metrics GetAverageMetrics() const;
            
            // Performance analysis
            std::vector<std::string> IdentifyBottlenecks() const;
            double GetSecurityOverhead() const;
            bool IsPerformanceOptimal() const;
            
            // Optimization suggestions
            std::vector<std::string> GetOptimizationSuggestions() const;
            void ApplyAutoOptimizations();
            
            // Real-time monitoring
            void StartRealTimeMonitoring();
            void StopRealTimeMonitoring();
            
            // Memory tracking
            void TrackAllocation(size_t size);
            void TrackDeallocation(size_t size);
            size_t GetCurrentMemoryUsage() const;
            
            // CPU tracking
            void UpdateCPUMetrics();
            double GetCPUUsage() const;
            
            // Export/Import
            bool ExportMetrics(const std::string& filename) const;
            bool ImportMetrics(const std::string& filename);

        private:
            PerformanceMonitor();
            ~PerformanceMonitor();
            PerformanceMonitor(const PerformanceMonitor&) = delete;
            PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;

            // Internal monitoring
            void MonitoringThreadFunc();
            void CollectSystemMetrics();
            void AnalyzePerformancePatterns();
            
            // Platform-specific implementations
            void InitializePlatformSpecific();
            void UpdatePlatformSpecificMetrics();
            
            // Static instance
            static std::unique_ptr<PerformanceMonitor> s_instance;
            static std::mutex s_instanceMutex;

            // State management
            std::atomic<bool> m_isInitialized{false};
            std::atomic<bool> m_isMonitoring{false};
            std::mutex m_metricsMutex;
            std::mutex m_profileMutex;
            
            // Monitoring thread
            std::thread m_monitoringThread;
            
            // Current metrics
            Metrics m_currentMetrics;
            std::array<Metrics, 60> m_metricsHistory; // Last 60 seconds
            size_t m_historyIndex = 0;
            
            // Profiling data
            std::map<std::string, std::vector<ProfilePoint>> m_profileData;
            std::map<std::string, std::chrono::high_resolution_clock::time_point> m_activeProfiles;
            
            // Memory tracking
            std::atomic<size_t> m_currentMemoryUsage{0};
            std::atomic<size_t> m_peakMemoryUsage{0};
            std::atomic<uint32_t> m_allocationCount{0};
            std::atomic<uint32_t> m_deallocationCount{0};
            
            // CPU tracking
            std::atomic<double> m_cpuUsage{0.0};
            std::chrono::steady_clock::time_point m_lastCpuUpdate;
            
            // Platform-specific data
            #ifdef _WIN32
            void* m_processHandle = nullptr;
            uint64_t m_lastKernelTime = 0;
            uint64_t m_lastUserTime = 0;
            uint64_t m_lastSystemTime = 0;
            #endif
        };

        /**
         * @brief RAII Performance Profiler
         * @details Automatic profiling with zero overhead when disabled
         */
        class ScopedProfiler {
        public:
            explicit ScopedProfiler(const std::string& name) 
                : m_name(name), m_enabled(true) {
                if (m_enabled) {
                    PerformanceMonitor::GetInstance().StartProfiling(m_name);
                }
            }
            
            ~ScopedProfiler() {
                if (m_enabled) {
                    PerformanceMonitor::GetInstance().EndProfiling(m_name);
                }
            }
            
            ScopedProfiler(const ScopedProfiler&) = delete;
            ScopedProfiler& operator=(const ScopedProfiler&) = delete;
            
        private:
            std::string m_name;
            bool m_enabled;
        };

        // Helper macros for easy profiling
        #ifdef RELEASE_BUILD
        #define PROFILE_SCOPE(name) ScopedProfiler _prof(name)
        #define PROFILE_FUNCTION() ScopedProfiler _prof(__FUNCTION__)
        #else
        #define PROFILE_SCOPE(name) (void)0
        #define PROFILE_FUNCTION() (void)0
        #endif

        // Memory tracking macros
        #define TRACK_MEMORY_ALLOC(size) \
            PerformanceMonitor::GetInstance().TrackAllocation(size)
            
        #define TRACK_MEMORY_DEALLOC(size) \
            PerformanceMonitor::GetInstance().TrackDeallocation(size)

    } // namespace Performance
} // namespace AetherVisor