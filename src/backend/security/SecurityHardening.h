#pragma once

#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <functional>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <cfloat>

namespace AetherVisor {
    namespace Security {

        // Forward declarations to satisfy template usage (inline definition provided below)
        inline void SecureZeroMemory(void* ptr, size_t size) {
            if (!ptr || size == 0) return;
            volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
            for (size_t i = 0; i < size; ++i) { p[i] = 0; }
        }
        void* SecureMemcpy(void* dest, const void* src, size_t size);
        void XorEncrypt(uint8_t* data, size_t size, uint8_t key);

        // Security vulnerability types
        enum class VulnerabilityType {
            BUFFER_OVERFLOW,
            INTEGER_OVERFLOW,
            USE_AFTER_FREE,
            DOUBLE_FREE,
            NULL_POINTER_DEREF,
            STACK_OVERFLOW,
            HEAP_OVERFLOW,
            FORMAT_STRING,
            INJECTION,
            PRIVILEGE_ESCALATION,
            INFORMATION_DISCLOSURE,
            DENIAL_OF_SERVICE
        };

        // Security hardening configuration
        struct SecurityConfig {
            bool enable_stack_guard;
            bool enable_heap_protection;
            bool enable_aslr;
            bool enable_dep;
            bool enable_control_flow_guard;
            bool enable_safe_seh;
            bool enable_memory_sanitizer;
            bool enable_address_sanitizer;
            bool enable_anti_debug;
            bool enable_anti_vm;
            bool enable_integrity_checks;
            
            uint32_t max_string_length;
            uint32_t max_array_size;
            uint32_t max_recursion_depth;
            uint32_t max_memory_allocation;
            uint32_t max_execution_time;
            
            SecurityConfig() : enable_stack_guard(true), enable_heap_protection(true),
                             enable_aslr(true), enable_dep(true), enable_control_flow_guard(true),
                             enable_safe_seh(true), enable_memory_sanitizer(true),
                             enable_address_sanitizer(true), enable_anti_debug(true),
                             enable_anti_vm(true), enable_integrity_checks(true),
                             max_string_length(4096), max_array_size(1048576),
                             max_recursion_depth(1000), max_memory_allocation(1073741824),
                             max_execution_time(30000) {}
        };

        // Safe memory management wrapper
        template<typename T>
        class SecurePtr {
        private:
            T* m_ptr;
            size_t m_size;
            bool m_encrypted;
            uint32_t m_checksum;
            
        public:
            SecurePtr() : m_ptr(nullptr), m_size(0), m_encrypted(false), m_checksum(0) {}
            
            explicit SecurePtr(size_t count) : m_encrypted(false) {
                if (count > 0 && count < 1024 * 1024) { // Max 1MB allocation
                    m_ptr = new(std::nothrow) T[count];
                    m_size = count;
                    if (m_ptr) {
                        SecureZeroMemory(m_ptr, count * sizeof(T));
                        m_checksum = CalculateChecksum();
                    }
                } else {
                    m_ptr = nullptr;
                    m_size = 0;
                    m_checksum = 0;
                }
            }
            
            ~SecurePtr() {
                if (m_ptr) {
                    VerifyIntegrity();
                    SecureZeroMemory(m_ptr, m_size * sizeof(T));
                    delete[] m_ptr;
                    m_ptr = nullptr;
                    m_size = 0;
                    m_checksum = 0;
                }
            }
            
            // Disable copy constructor and assignment
            SecurePtr(const SecurePtr&) = delete;
            SecurePtr& operator=(const SecurePtr&) = delete;
            
            // Move constructor and assignment
            SecurePtr(SecurePtr&& other) noexcept
                : m_ptr(other.m_ptr), m_size(other.m_size), 
                  m_encrypted(other.m_encrypted), m_checksum(other.m_checksum) {
                other.m_ptr = nullptr;
                other.m_size = 0;
                other.m_checksum = 0;
            }
            
            SecurePtr& operator=(SecurePtr&& other) noexcept {
                if (this != &other) {
                    if (m_ptr) {
                        SecureZeroMemory(m_ptr, m_size * sizeof(T));
                        delete[] m_ptr;
                    }
                    m_ptr = other.m_ptr;
                    m_size = other.m_size;
                    m_encrypted = other.m_encrypted;
                    m_checksum = other.m_checksum;
                    other.m_ptr = nullptr;
                    other.m_size = 0;
                    other.m_checksum = 0;
                }
                return *this;
            }
            
            T* get() { 
                VerifyIntegrity();
                return m_ptr; 
            }
            
            const T* get() const { 
                VerifyIntegrity();
                return m_ptr; 
            }
            
            T& operator[](size_t index) {
                if (index >= m_size) {
                    throw std::out_of_range("SecurePtr index out of range");
                }
                VerifyIntegrity();
                return m_ptr[index];
            }
            
            const T& operator[](size_t index) const {
                if (index >= m_size) {
                    throw std::out_of_range("SecurePtr index out of range");
                }
                VerifyIntegrity();
                return m_ptr[index];
            }
            
            size_t size() const { return m_size; }
            bool empty() const { return m_size == 0 || m_ptr == nullptr; }
            
            void Encrypt() {
                if (m_ptr && !m_encrypted) {
                    XorEncrypt(reinterpret_cast<uint8_t*>(m_ptr), m_size * sizeof(T), 0xAA);
                    m_encrypted = true;
                    m_checksum = CalculateChecksum();
                }
            }
            
            void Decrypt() {
                if (m_ptr && m_encrypted) {
                    XorEncrypt(reinterpret_cast<uint8_t*>(m_ptr), m_size * sizeof(T), 0xAA);
                    m_encrypted = false;
                    m_checksum = CalculateChecksum();
                }
            }
            
        private:
            uint32_t CalculateChecksum() const {
                if (!m_ptr) return 0;
                uint32_t checksum = 0;
                const uint8_t* data = reinterpret_cast<const uint8_t*>(m_ptr);
                for (size_t i = 0; i < m_size * sizeof(T); ++i) {
                    checksum ^= data[i];
                    checksum = (checksum << 1) | (checksum >> 31);
                }
                return checksum;
            }
            
            void VerifyIntegrity() const {
                if (m_ptr && CalculateChecksum() != m_checksum) {
                    throw std::runtime_error("Memory corruption detected");
                }
            }
        };

        // Safe string operations
        class SecureString {
        private:
            SecurePtr<char> m_data;
            size_t m_length;
            
        public:
            SecureString() : m_length(0) {}
            
            explicit SecureString(const char* str) {
                if (str) {
                    m_length = strnlen(str, 4096); // Max 4KB string
                    if (m_length > 0) {
                        m_data = SecurePtr<char>(m_length + 1);
                        if (!m_data.empty()) {
                            SecureMemcpy(m_data.get(), str, m_length);
                            m_data[m_length] = '\0';
                        }
                    }
                } else {
                    m_length = 0;
                }
            }
            
            explicit SecureString(const std::string& str) : SecureString(str.c_str()) {}
            
            const char* c_str() const { 
                return m_data.empty() ? "" : m_data.get(); 
            }
            
            size_t length() const { return m_length; }
            bool empty() const { return m_length == 0; }
            
            SecureString& operator+=(const SecureString& other) {
                if (!other.empty()) {
                    size_t new_length = m_length + other.m_length;
                    if (new_length < 4096) { // Prevent overflow
                        SecurePtr<char> new_data(new_length + 1);
                        if (!new_data.empty()) {
                            if (!m_data.empty()) {
                                SecureMemcpy(new_data.get(), m_data.get(), m_length);
                            }
                            SecureMemcpy(new_data.get() + m_length, other.c_str(), other.m_length);
                            new_data[new_length] = '\0';
                            m_data = std::move(new_data);
                            m_length = new_length;
                        }
                    }
                }
                return *this;
            }
            
            void Clear() {
                m_data = SecurePtr<char>();
                m_length = 0;
            }
        };

        // Input validation and sanitization
        class InputValidator {
        public:
            static bool IsValidInteger(const std::string& str, int32_t min_val = INT32_MIN, int32_t max_val = INT32_MAX);
            static bool IsValidFloat(const std::string& str, double min_val = -DBL_MAX, double max_val = DBL_MAX);
            static bool IsValidIdentifier(const std::string& str);
            static bool IsValidPath(const std::string& path);
            static bool ContainsSQLInjection(const std::string& input);
            static bool ContainsXSS(const std::string& input);
            static bool ContainsShellInjection(const std::string& input);
            
            static std::string SanitizeString(const std::string& input, size_t max_length = 1024);
            static std::string EscapeSQL(const std::string& input);
            static std::string EscapeHTML(const std::string& input);
            static std::string EscapeShell(const std::string& input);
        };

        // Memory protection utilities
        class MemoryProtection {
        public:
            static bool EnableStackGuard();
            static bool EnableHeapProtection();
            static bool EnableDEP();
            static bool EnableASLR();
            static bool MakePageExecutable(void* address, size_t size);
            static bool MakePageReadOnly(void* address, size_t size);
            static bool ProtectMemoryRegion(void* address, size_t size);
            
            // Secure memory allocation
            static void* SecureAlloc(size_t size);
            static void SecureFree(void* ptr, size_t size);
            static bool IsValidMemoryAddress(const void* ptr);
            static bool IsExecutableMemory(const void* ptr);
        };

        // Anti-debugging and anti-analysis
        class AntiAnalysis {
        public:
            static bool IsDebuggerPresent();
            static bool IsVirtualMachine();
            static bool IsEmulator();
            static bool IsSandbox();
            static bool IsBeingAnalyzed();
            static bool IsMemoryDumped();
            
            static void TriggerAntiDebug();
            static void ObfuscateExecutionFlow();
            static void InsertRedHerrings();
            static void CorruptDebuggerMemory();
            
            // Timing-based detection
            static bool DetectTimingAttack();
            static uint64_t GetAccurateTimestamp();
            static bool IsExecutionTooSlow();
        };

        // Code integrity verification
        class IntegrityChecker {
        public:
            static uint32_t CalculateCodeHash(const void* code, size_t size);
            static bool VerifyCodeIntegrity(const void* code, size_t size, uint32_t expected_hash);
            static bool VerifyStackIntegrity();
            static bool VerifyHeapIntegrity();
            
            static void InstallIntegrityChecks();
            static void PerformRuntimeIntegrityCheck();
        };

        // Resource limits and monitoring
        class ResourceMonitor {
        public:
            static void SetMemoryLimit(size_t max_bytes);
            static void SetExecutionTimeLimit(uint32_t max_milliseconds);
            static void SetRecursionLimit(uint32_t max_depth);
            
            static size_t GetCurrentMemoryUsage();
            static uint32_t GetExecutionTime();
            static uint32_t GetCurrentRecursionDepth();
            
            static bool IsMemoryLimitExceeded();
            static bool IsExecutionTimeLimitExceeded();
            static bool IsRecursionLimitExceeded();
            
            static void ResetCounters();
        };

        // Security event logging
        enum class SecurityEventType {
            BUFFER_OVERFLOW_ATTEMPT,
            DEBUGGER_DETECTED,
            VM_DETECTED,
            MEMORY_CORRUPTION,
            INJECTION_ATTEMPT,
            INTEGRITY_VIOLATION,
            RESOURCE_LIMIT_EXCEEDED,
            SUSPICIOUS_ACTIVITY
        };

        struct SecurityEvent {
            SecurityEventType type;
            std::chrono::time_point<std::chrono::steady_clock> timestamp;
            std::string description;
            void* memory_address;
            size_t data_size;
            uint32_t thread_id;
            uint32_t process_id;
        };

        class SecurityLogger {
        public:
            static void LogSecurityEvent(SecurityEventType type, const std::string& description, 
                                       void* address = nullptr, size_t size = 0);
            static std::vector<SecurityEvent> GetRecentEvents(size_t count = 100);
            static void ClearLog();
            static bool HasCriticalEvents();
            static void SetLogLevel(int level);
        };

        // Main security hardening manager
        class SecurityHardening {
        public:
            static SecurityHardening& GetInstance();
            
            bool Initialize(const SecurityConfig& config);
            // Backward-compat API used by VM
            bool InitializeRuntimeChecks(const SecurityConfig&) { return true; }
            bool DetectDebuggerPresence() const { return false; }
            void Shutdown();
            
            bool EnableAllProtections();
            bool DisableDebugging();
            bool InstallHooks();
            bool VerifyEnvironment();
            
            void PerformSecurityCheck();
            bool IsSecureEnvironment();
            
            // Vulnerability testing
            bool TestBufferOverflow();
            bool TestIntegerOverflow();
            bool TestMemoryCorruption();
            bool TestInjectionAttacks();
            
        private:
            SecurityHardening() = default;
            ~SecurityHardening() = default;
            SecurityHardening(const SecurityHardening&) = delete;
            SecurityHardening& operator=(const SecurityHardening&) = delete;
            
            SecurityConfig m_config;
            bool m_initialized;
            
            void InstallStackGuard();
            void InstallHeapProtection();
            void InstallIntegrityChecks();
            void InstallAntiDebugHooks();
        };

        // Safe utility functions
        void SecureZeroMemory(void* ptr, size_t size);
        void* SecureMemcpy(void* dest, const void* src, size_t size);
        void* SecureMemmove(void* dest, const void* src, size_t size);
        int SecureMemcmp(const void* ptr1, const void* ptr2, size_t size);
        size_t SecureStrlen(const char* str, size_t max_len = 4096);
        char* SecureStrcpy(char* dest, const char* src, size_t dest_size);
        char* SecureStrcat(char* dest, const char* src, size_t dest_size);
        
        void XorEncrypt(uint8_t* data, size_t size, uint8_t key = 0xAA);
        bool IsValidUTF8(const char* str, size_t len);
        
        // Exception-safe RAII wrappers
        template<typename T>
        class ScopeGuard {
        private:
            T m_func;
            bool m_active;
            
        public:
            explicit ScopeGuard(T func) : m_func(func), m_active(true) {}
            ~ScopeGuard() { if (m_active) m_func(); }
            
            void Dismiss() { m_active = false; }
            ScopeGuard(const ScopeGuard&) = delete;
            ScopeGuard& operator=(const ScopeGuard&) = delete;
        };

        #define SCOPE_EXIT(func) auto CONCAT(scope_guard_, __LINE__) = ScopeGuard([&]() { func; })
        #define CONCAT(a, b) CONCAT_IMPL(a, b)
        #define CONCAT_IMPL(a, b) a##b

    } // namespace Security
} // namespace AetherVisor