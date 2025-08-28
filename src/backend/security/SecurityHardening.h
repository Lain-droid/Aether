#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <map>
#include <random>
#include <atomic>
#include <cstdint>

namespace AetherVisor {
    namespace Security {

        /**
         * @brief Comprehensive Security Hardening System
         * @details Advanced security system that provides:
         *          - Memory protection and canaries
         *          - Anti-debugging measures
         *          - Control flow protection
         *          - Secure random generation
         *          - Tampering detection
         */
        class SecurityHardening {
        public:
            enum class ProtectionType {
                read_only,
                no_access,
                guard_page
            };

            struct ProtectedRegion {
                void* address = nullptr;
                size_t size = 0;
                ProtectionType type = ProtectionType::read_only;
                bool isProtected = false;
                #ifdef _WIN32
                unsigned long originalProtection = 0;
                #endif
            };

            // Singleton access
            static SecurityHardening& GetInstance();

            // Core security functions
            bool Initialize();
            void Shutdown();

            // Memory protection
            bool ProtectMemoryRegion(void* address, size_t size, ProtectionType type);
            bool UnprotectMemoryRegion(void* address);
            
            // Stack protection
            bool ValidateStackCanary(uint64_t canary);
            uint64_t GenerateStackCanary();
            
            // Secure operations
            bool SecureMemoryCompare(const void* buf1, const void* buf2, size_t size);
            void SecureZeroMemory(void* ptr, size_t size);
            uint64_t GenerateSecureRandom();
            
            // Address validation
            bool IsAddressValid(void* address);
            
            // Encryption/Decryption (simple)
            std::vector<unsigned char> EncryptData(const std::vector<unsigned char>& data, 
                                                   const std::vector<unsigned char>& key);
            std::vector<unsigned char> DecryptData(const std::vector<unsigned char>& encryptedData, 
                                                   const std::vector<unsigned char>& key);
            
            // Security monitoring
            bool DetectTampering();
            void PerformIntegrityCheck();
            
        private:
            SecurityHardening();
            ~SecurityHardening();
            SecurityHardening(const SecurityHardening&) = delete;
            SecurityHardening& operator=(const SecurityHardening&) = delete;

            // Initialization helpers
            void InitializeSecureRandom();
            void EnableStackProtection();
            void EnableHeapProtection();
            void EnableControlFlowGuard();
            void InitializeMemoryProtection();
            void CleanupMemoryProtection();
            void SetupAntiDebugging();
            
            // Security countermeasures
            void TriggerSecurityCountermeasures();

            // Static instance management
            static std::unique_ptr<SecurityHardening> s_instance;
            static std::mutex s_instanceMutex;

            // Instance state
            std::atomic<bool> m_isInitialized{false};
            std::mutex m_protectionMutex;
            
            // Protected memory regions
            std::map<void*, ProtectedRegion> m_protectedRegions;
            
            // Stack canaries
            std::vector<uint64_t> m_canaryValues;
            
            // Secure random number generator
            std::mt19937_64 m_secureRandom;
        };

        // Helper macros for stack protection
        #define STACK_CANARY_DECLARE(name) \
            uint64_t name = SecurityHardening::GetInstance().GenerateStackCanary()

        #define STACK_CANARY_CHECK(canary) \
            do { \
                if (!SecurityHardening::GetInstance().ValidateStackCanary(canary)) { \
                    return false; \
                } \
            } while(0)

        #define SECURE_ZERO(ptr, size) \
            SecurityHardening::GetInstance().SecureZeroMemory(ptr, size)

        #define SECURE_COMPARE(buf1, buf2, size) \
            SecurityHardening::GetInstance().SecureMemoryCompare(buf1, buf2, size)

    } // namespace Security
} // namespace AetherVisor