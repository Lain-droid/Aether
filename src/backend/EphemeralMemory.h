#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>
#include <functional>
#include "AIController.h"

#ifdef _WIN32
#include <Windows.h>
using ByteType = BYTE;
#else
using ByteType = std::uint8_t;
#endif

namespace AetherVisor {
    namespace Payload {

        /**
         * @class EphemeralMemory
         * @brief AI-Enhanced temporary, self-destructing block of memory with intelligent obfuscation.
         *
         * Enhanced ephemeral memory system that integrates with AI for:
         * - Intelligent memory allocation patterns
         * - Adaptive obfuscation techniques
         * - Real-time threat-based memory management
         * - Synchronized destruction with AI risk assessment
         */
        class EphemeralMemory {
        public:
            enum class ObfuscationType {
                NONE,
                XOR_SIMPLE,
                XOR_DYNAMIC,
                AES_LIGHT,
                POLYMORPHIC,
                AI_ADAPTIVE
            };

            struct MemoryMetrics {
                uint64_t totalAllocations = 0;
                uint64_t totalDeallocations = 0;
                uint64_t scanDetections = 0;
                uint64_t obfuscationCycles = 0;
                std::chrono::milliseconds totalObfuscationTime{0};
            };

            /**
             * @brief AI-enhanced allocation with intelligent obfuscation
             */
            explicit EphemeralMemory(size_t size, std::shared_ptr<Backend::AIController> aiController = nullptr);

            /**
             * @brief Destructor with AI-synchronized secure erasure
             */
            ~EphemeralMemory();

            /**
             * @brief AI-guided obfuscated write
             */
            bool Write(const std::vector<ByteType>& data);
            bool WriteWithObfuscation(const std::vector<ByteType>& data, ObfuscationType type);

            /**
             * @brief AI-synchronized read with deobfuscation
             */
            std::vector<ByteType> Read(size_t size) const;
            std::vector<ByteType> ReadWithDeobfuscation(size_t size) const;

            /**
             * @brief AI-controlled memory access
             */
            void* GetAddress() const;
            size_t GetSize() const;

            // AI synchronization methods
            void SyncWithAI();
            void AdaptObfuscation();
            void TriggerEmergencyErase();
            bool DetectMemoryScanning();

            // Advanced memory management
            void RotateObfuscationKey();
            void ApplyDecoyPattern();
            void MimicLegitimateMemory();

            // Metrics and monitoring
            MemoryMetrics GetMetrics() const { return m_metrics; }
            double GetObfuscationStrength() const;
            bool IsCompromised() const;

            // Disable copy and assignment
            EphemeralMemory(const EphemeralMemory&) = delete;
            EphemeralMemory& operator=(const EphemeralMemory&) = delete;

            // Enable move semantics
            EphemeralMemory(EphemeralMemory&& other) noexcept;
            EphemeralMemory& operator=(EphemeralMemory&& other) noexcept;

        private:
            void* m_address = nullptr;
            size_t m_size = 0;
            std::shared_ptr<Backend::AIController> m_aiController;
            ObfuscationType m_currentObfuscation = ObfuscationType::NONE;
            std::vector<ByteType> m_obfuscationKey;
            std::chrono::steady_clock::time_point m_lastScan;
            MemoryMetrics m_metrics;
            mutable std::mutex m_memoryMutex;
            bool m_isObfuscated = false;
            uint32_t m_scanDetectionCount = 0;

            // AI-enhanced internal methods
            void InitializeObfuscation();
            void ApplyObfuscation(ObfuscationType type);
            void RemoveObfuscation();
            void GenerateDynamicKey();
            void SecureErase();
            bool ValidateIntegrity() const;
            void UpdateAIMetrics();
        };

    } // namespace Payload
} // namespace AetherVisor
