#include "SecurityHardening.h"
#include "../AIController.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#include <Wincrypt.h>
#pragma comment(lib, "Advapi32.lib")
#endif

namespace AetherVisor {
    namespace Security {

        // Static instance management
        std::unique_ptr<SecurityHardening> SecurityHardening::s_instance = nullptr;
        std::mutex SecurityHardening::s_instanceMutex;

        SecurityHardening& SecurityHardening::GetInstance() {
            std::lock_guard<std::mutex> lock(s_instanceMutex);
            if (!s_instance) {
                s_instance = std::unique_ptr<SecurityHardening>(new SecurityHardening());
            }
            return *s_instance;
        }

        SecurityHardening::SecurityHardening() {
            Initialize();
        }

        SecurityHardening::~SecurityHardening() {
            Shutdown();
        }

        bool SecurityHardening::Initialize() {
            try {
                // Initialize secure random number generator
                InitializeSecureRandom();
                
                // Enable stack protection
                EnableStackProtection();
                
                // Enable heap protection
                EnableHeapProtection();
                
                // Enable control flow guard
                EnableControlFlowGuard();
                
                // Initialize memory protection
                InitializeMemoryProtection();
                
                // Setup anti-debugging measures
                SetupAntiDebugging();
                
                m_isInitialized = true;
                return true;
            }
            catch (const std::exception& e) {
                return false;
            }
        }

        void SecurityHardening::Shutdown() {
            if (!m_isInitialized) return;
            
            // Clean up security measures
            CleanupMemoryProtection();
            m_isInitialized = false;
        }

        void SecurityHardening::InitializeSecureRandom() {
            #ifdef _WIN32
            // Use Windows Cryptographic API for secure random
            HCRYPTPROV hCryptProv;
            if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
                BYTE randomBytes[32];
                if (CryptGenRandom(hCryptProv, sizeof(randomBytes), randomBytes)) {
                    // Seed our random engine with cryptographically secure data
                    std::random_device rd;
                    std::seed_seq seed(randomBytes, randomBytes + sizeof(randomBytes));
                    m_secureRandom.seed(seed);
                }
                CryptReleaseContext(hCryptProv, 0);
            }
            #else
            // Use std::random_device on non-Windows platforms
            std::random_device rd;
            m_secureRandom.seed(rd());
            #endif
        }

        void SecurityHardening::EnableStackProtection() {
            #ifdef _WIN32
            // Stack protection is enabled by compiler flags in our build
            // Additional runtime checks can be added here
            
            // Enable DEP (Data Execution Prevention) if not already enabled
            DWORD flags = PROCESS_DEP_ENABLE | PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION;
            SetProcessDEPPolicy(flags);
            #endif
        }

        void SecurityHardening::EnableHeapProtection() {
            #ifdef _WIN32
            // Enable heap protection features
            HANDLE hHeap = GetProcessHeap();
            if (hHeap) {
                ULONG HeapInformation = 2; // Enable LFH (Low Fragmentation Heap)
                HeapSetInformation(hHeap, HeapCompatibilityInformation, &HeapInformation, sizeof(HeapInformation));
            }
            #endif
        }

        void SecurityHardening::EnableControlFlowGuard() {
            #ifdef _WIN32
            // Control Flow Guard is enabled by compiler flags
            // Additional runtime validation can be added here
            #endif
        }

        void SecurityHardening::InitializeMemoryProtection() {
            // Initialize memory protection structures
            m_protectedRegions.clear();
            m_canaryValues.clear();
            
            // Generate random canary values
            for (int i = 0; i < 16; ++i) {
                m_canaryValues.push_back(GenerateSecureRandom());
            }
        }

        void SecurityHardening::CleanupMemoryProtection() {
            // Clean up protected regions
            for (auto& region : m_protectedRegions) {
                if (region.second.isProtected) {
                    UnprotectMemoryRegion(region.first);
                }
            }
            m_protectedRegions.clear();
            
            // Clear canary values securely
            SecureZeroMemory(m_canaryValues.data(), m_canaryValues.size() * sizeof(uint64_t));
            m_canaryValues.clear();
        }

        void SecurityHardening::SetupAntiDebugging() {
            #ifdef _WIN32
            // Basic anti-debugging checks
            if (IsDebuggerPresent()) {
                // Debugger detected - could implement countermeasures
                Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::ANTI_CHEAT_PROBE);
            }
            
            // Check for remote debugger
            BOOL isRemoteDebuggerPresent = FALSE;
            CheckRemoteDebuggerPresent(GetCurrentProcess(), &isRemoteDebuggerPresent);
            if (isRemoteDebuggerPresent) {
                Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::BEHAVIORAL_ANALYSIS_DETECTED);
            }
            #endif
        }

        bool SecurityHardening::ProtectMemoryRegion(void* address, size_t size, ProtectionType type) {
            if (!address || size == 0) return false;
            
            std::lock_guard<std::mutex> lock(m_protectionMutex);
            
            ProtectedRegion region;
            region.address = address;
            region.size = size;
            region.type = type;
            region.isProtected = false;
            
            #ifdef _WIN32
            DWORD oldProtect;
            DWORD newProtect;
            
            switch (type) {
                case ProtectionType::READ_only:
                    newProtect = PAGE_READONLY;
                    break;
                case ProtectionType::no_access:
                    newProtect = PAGE_NOACCESS;
                    break;
                case ProtectionType::guard_page:
                    newProtect = PAGE_GUARD | PAGE_READWRITE;
                    break;
                default:
                    return false;
            }
            
            if (VirtualProtect(address, size, newProtect, &oldProtect)) {
                region.originalProtection = oldProtect;
                region.isProtected = true;
                m_protectedRegions[address] = region;
                return true;
            }
            #endif
            
            return false;
        }

        bool SecurityHardening::UnprotectMemoryRegion(void* address) {
            std::lock_guard<std::mutex> lock(m_protectionMutex);
            
            auto it = m_protectedRegions.find(address);
            if (it == m_protectedRegions.end()) return false;
            
            auto& region = it->second;
            if (!region.isProtected) return false;
            
            #ifdef _WIN32
            DWORD oldProtect;
            if (VirtualProtect(address, region.size, region.originalProtection, &oldProtect)) {
                region.isProtected = false;
                m_protectedRegions.erase(it);
                return true;
            }
            #endif
            
            return false;
        }

        bool SecurityHardening::ValidateStackCanary(uint64_t canary) {
            std::lock_guard<std::mutex> lock(m_protectionMutex);
            
            // Check if canary matches any of our valid canary values
            for (const auto& validCanary : m_canaryValues) {
                if (canary == validCanary) {
                    return true;
                }
            }
            
            // Stack corruption detected!
            Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::MEMORY_SCAN_EVASION);
            return false;
        }

        uint64_t SecurityHardening::GenerateStackCanary() {
            std::lock_guard<std::mutex> lock(m_protectionMutex);
            
            uint64_t canary = GenerateSecureRandom();
            m_canaryValues.push_back(canary);
            
            // Limit the number of active canaries
            if (m_canaryValues.size() > 32) {
                m_canaryValues.erase(m_canaryValues.begin());
            }
            
            return canary;
        }

        bool SecurityHardening::SecureMemoryCompare(const void* buf1, const void* buf2, size_t size) {
            if (!buf1 || !buf2 || size == 0) return false;
            
            // Constant-time memory comparison to prevent timing attacks
            const volatile unsigned char* p1 = static_cast<const volatile unsigned char*>(buf1);
            const volatile unsigned char* p2 = static_cast<const volatile unsigned char*>(buf2);
            
            volatile unsigned char result = 0;
            for (size_t i = 0; i < size; ++i) {
                result |= p1[i] ^ p2[i];
            }
            
            return result == 0;
        }

        void SecurityHardening::SecureZeroMemory(void* ptr, size_t size) {
            if (!ptr || size == 0) return;
            
            #ifdef _WIN32
            // Use Windows secure zero function
            ::SecureZeroMemory(ptr, size);
            #else
            // Prevent compiler optimization of memset
            volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
            for (size_t i = 0; i < size; ++i) {
                p[i] = 0;
            }
            #endif
        }

        uint64_t SecurityHardening::GenerateSecureRandom() {
            std::uniform_int_distribution<uint64_t> dist;
            return dist(m_secureRandom);
        }

        bool SecurityHardening::IsAddressValid(void* address) {
            if (!address) return false;
            
            #ifdef _WIN32
            // Check if address is in valid memory region
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(address, &mbi, sizeof(mbi)) == 0) {
                return false;
            }
            
            return (mbi.State == MEM_COMMIT) && 
                   (mbi.Protect != PAGE_NOACCESS) && 
                   (mbi.Protect != PAGE_GUARD);
            #else
            // Basic null check for non-Windows
            return true;
            #endif
        }

        std::vector<unsigned char> SecurityHardening::EncryptData(const std::vector<unsigned char>& data, const std::vector<unsigned char>& key) {
            if (data.empty() || key.empty()) return {};
            
            std::vector<unsigned char> encrypted = data;
            
            // Simple XOR encryption with key rotation (not production-grade)
            for (size_t i = 0; i < encrypted.size(); ++i) {
                encrypted[i] ^= key[i % key.size()];
                encrypted[i] ^= static_cast<unsigned char>(GenerateSecureRandom() & 0xFF);
            }
            
            return encrypted;
        }

        std::vector<unsigned char> SecurityHardening::DecryptData(const std::vector<unsigned char>& encryptedData, const std::vector<unsigned char>& key) {
            // This is a placeholder - actual implementation would reverse the encryption
            return encryptedData;
        }

        bool SecurityHardening::DetectTampering() {
            // Check for various tampering indicators
            bool tamperingDetected = false;
            
            #ifdef _WIN32
            // Check for debugger presence
            if (IsDebuggerPresent()) {
                tamperingDetected = true;
            }
            
            // Check for breakpoints in our code
            // This is a simplified check
            #endif
            
            // Check stack canaries
            std::lock_guard<std::mutex> lock(m_protectionMutex);
            if (m_canaryValues.empty()) {
                tamperingDetected = true;
            }
            
            if (tamperingDetected) {
                Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::ANTI_CHEAT_PROBE);
            }
            
            return tamperingDetected;
        }

        void SecurityHardening::PerformIntegrityCheck() {
            // Perform various integrity checks
            if (DetectTampering()) {
                // Integrity violation detected
                Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::SIGNATURE_SCAN_ATTEMPT);
                
                // Could implement countermeasures here
                TriggerSecurityCountermeasures();
            }
        }

        void SecurityHardening::TriggerSecurityCountermeasures() {
            // Implement security countermeasures
            
            // Generate new canary values
            std::lock_guard<std::mutex> lock(m_protectionMutex);
            m_canaryValues.clear();
            for (int i = 0; i < 16; ++i) {
                m_canaryValues.push_back(GenerateSecureRandom());
            }
            
            // Notify AI system
            Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::ADAPTIVE_BEHAVIOR_CHANGE);
        }

    } // namespace Security
} // namespace AetherVisor