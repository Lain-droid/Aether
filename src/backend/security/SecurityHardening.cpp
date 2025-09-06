#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "SecurityHardening.h"
#include "SecurityTypes.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <regex>
#include <thread>
#include <mutex>
#include <cmath>
#include "XorStr.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <intrin.h>
#else
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#endif

namespace AetherVisor {
    namespace Security {

        static std::mutex g_security_mutex;
        static std::vector<SecurityEvent> g_security_events;
        static SecurityConfig g_current_config;
        static size_t g_memory_usage = 0;
        static std::chrono::time_point<std::chrono::steady_clock> g_start_time;
        static uint32_t g_recursion_depth = 0;

        // SecurityConfig defaulted in header

        // InputValidator implementation
        bool InputValidator::IsValidInteger(const std::string& str, int32_t min_val, int32_t max_val) {
            if (str.empty() || str.length() > 32) return false;
            
            try {
                size_t pos;
                long long val = std::stoll(str, &pos);
                if (pos != str.length()) return false;
                return val >= min_val && val <= max_val;
            } catch (...) {
                return false;
            }
        }

        bool InputValidator::IsValidFloat(const std::string& str, double min_val, double max_val) {
            if (str.empty() || str.length() > 32) return false;
            
            try {
                size_t pos;
                double val = std::stod(str, &pos);
                if (pos != str.length()) return false;
                return val >= min_val && val <= max_val && std::isfinite(val);
            } catch (...) {
                return false;
            }
        }

        bool InputValidator::IsValidIdentifier(const std::string& str) {
            if (str.empty() || str.length() > 256) return false;
            
            if (!std::isalpha(str[0]) && str[0] != '_') return false;
            
            return std::all_of(str.begin() + 1, str.end(), 
                [](char c) { return std::isalnum(c) || c == '_'; });
        }

        bool InputValidator::IsValidPath(const std::string& path) {
            if (path.empty() || path.length() > 1024) return false;
            
            // Check for path traversal attacks
            if (path.find("..") != std::string::npos) return false;
            if (path.find("//") != std::string::npos) return false;
            
            // Check for dangerous characters
            const char* dangerous_chars = "<>:\"|?*\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
            return path.find_first_of(dangerous_chars) == std::string::npos;
        }

        bool InputValidator::ContainsSQLInjection(const std::string& input) {
            static const std::vector<std::string> sql_keywords = {
                "select", "insert", "update", "delete", "drop", "create", "alter",
                "union", "exec", "execute", "xp_", "sp_", "script", "javascript"
            };
            
            std::string lower_input = input;
            std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), ::tolower);
            
            for (const auto& keyword : sql_keywords) {
                if (lower_input.find(keyword) != std::string::npos) {
                    return true;
                }
            }
            
            // Check for SQL injection patterns
            if (input.find("'") != std::string::npos && input.find("--") != std::string::npos) return true;
            if (input.find("/*") != std::string::npos && input.find("*/") != std::string::npos) return true;
            
            return false;
        }

        bool InputValidator::ContainsXSS(const std::string& input) {
            static const std::vector<std::string> xss_patterns = {
                "<script", "</script>", "javascript:", "vbscript:", "onload=", "onerror=",
                "onclick=", "onmouseover=", "onfocus=", "onblur=", "alert(", "document.",
                "window.", "eval(", "expression(", "url(", "@import"
            };
            
            std::string lower_input = input;
            std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), ::tolower);
            
            for (const auto& pattern : xss_patterns) {
                if (lower_input.find(pattern) != std::string::npos) {
                    return true;
                }
            }
            
            return false;
        }

        bool InputValidator::ContainsShellInjection(const std::string& input) {
            static const std::vector<std::string> shell_patterns = {
                "|", "&", ";", "`", "$", "(", ")", "{", "}", "[", "]",
                "&&", "||", ">>", "<<", "../", "./", "/bin/", "/usr/",
                "cmd", "powershell", "bash", "sh", "perl", "python",
                "wget", "curl", "nc", "netcat", "telnet", "ssh"
            };
            
            for (const auto& pattern : shell_patterns) {
                if (input.find(pattern) != std::string::npos) {
                    return true;
                }
            }
            
            return false;
        }

        std::string InputValidator::SanitizeString(const std::string& input, size_t max_length) {
            std::string result;
            result.reserve(std::min(input.length(), max_length));
            
            for (char c : input) {
                if (result.length() >= max_length) break;
                
                // Only allow printable ASCII characters and common whitespace
                if ((c >= 32 && c <= 126) || c == '\t' || c == '\n' || c == '\r') {
                    result += c;
                }
            }
            
            return result;
        }

        std::string InputValidator::EscapeSQL(const std::string& input) {
            std::string result;
            result.reserve(input.length() * 2);
            
            for (char c : input) {
                if (c == '\'') {
                    result += "''";
                } else if (c == '"') {
                    result += "\"\"";
                } else if (c == '\\') {
                    result += "\\\\";
                } else {
                    result += c;
                }
            }
            
            return result;
        }

        std::string InputValidator::EscapeHTML(const std::string& input) {
            std::string result;
            result.reserve(input.length() * 2);
            
            for (char c : input) {
                switch (c) {
                    case '<': result += "&lt;"; break;
                    case '>': result += "&gt;"; break;
                    case '&': result += "&amp;"; break;
                    case '"': result += "&quot;"; break;
                    case '\'': result += "&#x27;"; break;
                    default: result += c; break;
                }
            }
            
            return result;
        }

        // MemoryProtection implementation
        bool MemoryProtection::EnableStackGuard() {
#ifdef _WIN32
            // Windows: Enable stack guard through compiler flags and runtime checks
            return true;
#else
            // Linux: Stack guard is typically enabled by default with modern compilers
            return true;
#endif
        }

        bool MemoryProtection::EnableHeapProtection() {
#ifdef _WIN32
            // Enable heap protection flags
            HANDLE heap = GetProcessHeap();
            return HeapSetInformation(heap, HeapEnableTerminationOnCorruption, nullptr, 0) != 0;
#else
            // Linux: Use malloc_check_ environment variable or tcmalloc
            setenv("MALLOC_CHECK_", "3", 1);
            return true;
#endif
        }

        bool MemoryProtection::EnableDEP() {
#ifdef _WIN32
            typedef BOOL (WINAPI *SetProcessDEPPolicyFunc)(DWORD);
            HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
            if (kernel32) {
                SetProcessDEPPolicyFunc SetProcessDEPPolicy = 
                    (SetProcessDEPPolicyFunc)GetProcAddress(kernel32, "SetProcessDEPPolicy");
                if (SetProcessDEPPolicy) {
                    return SetProcessDEPPolicy(PROCESS_DEP_ENABLE) != 0;
                }
            }
            return false;
#else
            // Linux: NX bit protection is typically enabled by default
            return true;
#endif
        }

        bool MemoryProtection::EnableASLR() {
#ifdef _WIN32
            // ASLR is enabled by default on modern Windows
            return true;
#else
            // Linux: Check if ASLR is enabled
            FILE* f = fopen("/proc/sys/kernel/randomize_va_space", "r");
            if (f) {
                int value;
                if (fscanf(f, "%d", &value) == 1) {
                    fclose(f);
                    return value >= 1;
                }
                fclose(f);
            }
            return false;
#endif
        }

        void* MemoryProtection::SecureAlloc(size_t size) {
            if (size == 0 || size > g_current_config.max_memory_allocation) {
                return nullptr;
            }
            
            std::lock_guard<std::mutex> lock(g_security_mutex);
            if (g_memory_usage + size > g_current_config.max_memory_allocation) {
                SecurityLogger::LogSecurityEvent(SecurityEventType::RESOURCE_LIMIT_EXCEEDED,
                    XorS("Memory allocation limit exceeded"));
                return nullptr;
            }
            
#ifdef _WIN32
            void* ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (ptr) {
                SecureZeroMemory(ptr, size);
                g_memory_usage += size;
            }
            return ptr;
#else
            void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (ptr != MAP_FAILED) {
                memset(ptr, 0, size);
                g_memory_usage += size;
                return ptr;
            }
            return nullptr;
#endif
        }

        void MemoryProtection::SecureFree(void* ptr, size_t size) {
            if (!ptr) return;
            
            SecureZeroMemory(ptr, size);
            
            std::lock_guard<std::mutex> lock(g_security_mutex);
            g_memory_usage = (g_memory_usage >= size) ? g_memory_usage - size : 0;
            
#ifdef _WIN32
            VirtualFree(ptr, 0, MEM_RELEASE);
#else
            munmap(ptr, size);
#endif
        }

        // AntiAnalysis implementation
        bool AntiAnalysis::IsDebuggerPresent() {
#ifdef _WIN32
            // Multiple debugger detection techniques
            if (::IsDebuggerPresent()) return true;
            
            // Check PEB flag
            // __asm not supported on x64 MSVC. Use IsDebuggerPresent/CheckRemoteDebuggerPresent already above.
            // PEB check skipped in x64 build to maintain compatibility.
            
            // Check for remote debugger
            BOOL remote_debugger = FALSE;
            CheckRemoteDebuggerPresent(GetCurrentProcess(), &remote_debugger);
            if (remote_debugger) return true;
            
            // Timing check
            DWORD start = GetTickCount();
            Sleep(10);
            DWORD end = GetTickCount();
            if (end - start > 50) return true; // Debugger slowing execution
            
            return false;
#else
            // Linux debugger detection
            FILE* f = fopen("/proc/self/status", "r");
            if (f) {
                char line[256];
                while (fgets(line, sizeof(line), f)) {
                    if (strncmp(line, "TracerPid:", 10) == 0) {
                        int tracer_pid = atoi(line + 10);
                        fclose(f);
                        return tracer_pid != 0;
                    }
                }
                fclose(f);
            }
            return false;
#endif
        }

        bool AntiAnalysis::IsVirtualMachine() {
#ifdef _WIN32
            // Check for VM artifacts
            const wchar_t* vm_files[] = {
                L"C:\\windows\\system32\\drivers\\vmmouse.sys",
                L"C:\\windows\\system32\\drivers\\vmhgfs.sys",
                L"C:\\windows\\system32\\drivers\\VBoxMouse.sys",
                L"C:\\windows\\system32\\drivers\\VBoxGuest.sys",
                L"C:\\windows\\system32\\drivers\\VBoxSF.sys",
                L"C:\\windows\\system32\\vboxdisp.dll",
                L"C:\\windows\\system32\\vboxhook.dll",
                L"C:\\windows\\system32\\vboxoglerrorspu.dll",
                L"C:\\windows\\system32\\vboxoglpassthroughspu.dll",
                L"C:\\windows\\system32\\vboxservice.exe",
                L"C:\\windows\\system32\\vboxtray.exe",
                L"C:\\windows\\system32\\VBoxControl.exe"
            };
            
            for (const auto& file : vm_files) {
                if (GetFileAttributesW(file) != INVALID_FILE_ATTRIBUTES) {
                    return true;
                }
            }
            
            // Check registry keys
            HKEY key;
            const wchar_t* vm_keys[] = {
                L"SOFTWARE\\VMware, Inc.\\VMware Tools",
                L"SOFTWARE\\Oracle\\VirtualBox Guest Additions",
                L"HARDWARE\\DESCRIPTION\\System\\SystemBiosInformation"
            };
            
            for (const auto& vm_key : vm_keys) {
                if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, vm_key, 0, KEY_READ, &key) == ERROR_SUCCESS) {
                    RegCloseKey(key);
                    return true;
                }
            }
            
            // CPUID check for virtualization
            int cpuid_result[4];
            __cpuid(cpuid_result, 1);
            if (cpuid_result[2] & (1 << 31)) { // Hypervisor present bit
                return true;
            }
            
            return false;
#else
            // Linux VM detection
            FILE* f = fopen("/proc/cpuinfo", "r");
            if (f) {
                char line[256];
                while (fgets(line, sizeof(line), f)) {
                    if (strstr(line, "hypervisor") || strstr(line, "VMware") || 
                        strstr(line, "VirtualBox") || strstr(line, "QEMU")) {
                        fclose(f);
                        return true;
                    }
                }
                fclose(f);
            }
            
            // Check for VM-specific files
            const char* vm_files[] = {
                "/proc/vz", "/proc/bc", "/proc/xen", "/sys/bus/pci/devices/0000:00:04.0/vendor"
            };
            
            for (const auto& file : vm_files) {
                if (access(file, F_OK) == 0) {
                    return true;
                }
            }
            
            return false;
#endif
        }

        void AntiAnalysis::TriggerAntiDebug() {
            if (IsDebuggerPresent()) {
                SecurityLogger::LogSecurityEvent(SecurityEventType::DEBUGGER_DETECTED, 
                    XorS("Debugger detected - triggering countermeasures"));
                
                // Corrupt debugger memory
                CorruptDebuggerMemory();
                
                // Exit gracefully to avoid detection
                std::exit(1);
            }
        }

        void AntiAnalysis::CorruptDebuggerMemory() {
#ifdef _WIN32
            // Attempt to corrupt common debugger memory regions
            const uintptr_t debugger_processes[] = { 0x1000, 0x2000, 0x3000, 0x4000 };
            
            for (uintptr_t addr : debugger_processes) {
                __try {
                    *(volatile DWORD*)addr = 0xDEADBEEF;
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    // Ignore access violations
                }
            }
#endif
        }

        // SecurityLogger implementation
        void SecurityLogger::LogSecurityEvent(SecurityEventType type, const std::string& description, 
                                            void* address, size_t size) {
            std::lock_guard<std::mutex> lock(g_security_mutex);
            
            SecurityEvent event(SecurityEventType::DEBUGGER_DETECTED, "Critical security violation detected");
            event.type = type;
            event.timestamp = std::chrono::system_clock::now();
            event.description = description;
            event.memory_address = address;
            event.data_size = size;
            event.thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
#ifdef _WIN32
            event.process_id = GetCurrentProcessId();
#else
            event.process_id = getpid();
#endif
            
            g_security_events.push_back(event);
            
            // Keep only recent events (last 1000)
            if (g_security_events.size() > 1000) {
                g_security_events.erase(g_security_events.begin(), 
                                      g_security_events.begin() + 500);
            }
        }

        std::vector<SecurityEvent> SecurityLogger::GetRecentEvents(size_t count) {
            std::lock_guard<std::mutex> lock(g_security_mutex);
            
            if (count >= g_security_events.size()) {
                return g_security_events;
            }
            
            return std::vector<SecurityEvent>(
                g_security_events.end() - count, g_security_events.end());
        }

        bool SecurityLogger::HasCriticalEvents() {
            std::lock_guard<std::mutex> lock(g_security_mutex);
            
            auto recent_time = std::chrono::system_clock::now() - std::chrono::minutes(5);
            
            return std::any_of(g_security_events.begin(), g_security_events.end(),
                [recent_time](const SecurityEvent& event) {
                    return event.timestamp > recent_time && 
                           (event.type == SecurityEventType::DEBUGGER_DETECTED ||
                            event.type == SecurityEventType::MEMORY_CORRUPTION ||
                            event.type == SecurityEventType::INJECTION_ATTEMPT);
                });
        }

        // SecurityHardening implementation
        SecurityHardening& SecurityHardening::GetInstance() {
            static SecurityHardening instance;
            return instance;
        }

        bool SecurityHardening::Initialize(const SecurityConfig& config) {
            std::lock_guard<std::mutex> lock(g_security_mutex);
            
            m_config = config;
            g_current_config = config;
            g_start_time = std::chrono::steady_clock::now();
            
            if (config.enable_stack_guard) {
                InstallStackGuard();
            }
            
            if (config.enable_heap_protection) {
                InstallHeapProtection();
            }
            
            if (config.enable_anti_debug) {
                InstallAntiDebugHooks();
            }
            
            if (config.enable_integrity_checks) {
                InstallIntegrityChecks();
            }
            
            m_initialized = true;
            return true;
        }

        bool SecurityHardening::EnableAllProtections() {
            return MemoryProtection::EnableStackGuard() &&
                   MemoryProtection::EnableHeapProtection() &&
                   MemoryProtection::EnableDEP() &&
                   MemoryProtection::EnableASLR();
        }

        bool SecurityHardening::VerifyEnvironment() {
            if (AntiAnalysis::IsDebuggerPresent()) {
                SecurityLogger::LogSecurityEvent(SecurityEventType::DEBUGGER_DETECTED,
                    XorS("Debugger detected in environment verification"));
                return false;
            }
            
            if (AntiAnalysis::IsVirtualMachine()) {
                SecurityLogger::LogSecurityEvent(SecurityEventType::VM_DETECTED,
                    XorS("Virtual machine detected"));
                return false;
            }
            
            return true;
        }

        void SecurityHardening::PerformSecurityCheck() {
            if (!m_initialized) return;
            
            // Check for debugger
            if (m_config.enable_anti_debug && AntiAnalysis::IsDebuggerPresent()) {
                AntiAnalysis::TriggerAntiDebug();
            }
            
            // Check resource limits
            if (ResourceMonitor::IsMemoryLimitExceeded()) {
                SecurityLogger::LogSecurityEvent(SecurityEventType::RESOURCE_LIMIT_EXCEEDED,
                    XorS("Memory limit exceeded"));
            }
            
            if (ResourceMonitor::IsExecutionTimeLimitExceeded()) {
                SecurityLogger::LogSecurityEvent(SecurityEventType::RESOURCE_LIMIT_EXCEEDED,
                    XorS("Execution time limit exceeded"));
            }
            
            // Perform integrity checks
            if (m_config.enable_integrity_checks) {
                IntegrityChecker::PerformRuntimeIntegrityCheck();
            }
        }

        // Utility functions

        void* SecureMemcpy(void* dest, const void* src, size_t size) {
            if (!dest || !src || size == 0) return dest;
            
            // Check for buffer overlap
            if (dest == src) return dest;
            
            const char* src_ptr = static_cast<const char*>(src);
            char* dest_ptr = static_cast<char*>(dest);
            
            // Check for potential overflow
            if (src_ptr < dest_ptr && src_ptr + size > dest_ptr) {
                SecurityLogger::LogSecurityEvent(SecurityEventType::BUFFER_OVERFLOW_ATTEMPT,
                    XorS("Buffer overlap detected in SecureMemcpy"));
                return dest;
            }
            
            return std::memcpy(dest, src, size);
        }

        void XorEncrypt(uint8_t* data, size_t size, uint8_t key) {
            if (!data || size == 0) return;
            
            for (size_t i = 0; i < size; ++i) {
                data[i] ^= key;
            }
        }

        // Stub implementations for complex functions
        void SecurityHardening::InstallStackGuard() {}
        void SecurityHardening::InstallHeapProtection() {}
        void SecurityHardening::InstallIntegrityChecks() {}
        void SecurityHardening::InstallAntiDebugHooks() {}
        
        bool MemoryProtection::MakePageExecutable(void* address, size_t size) { return true; }
        bool MemoryProtection::MakePageReadOnly(void* address, size_t size) { return true; }
        bool MemoryProtection::ProtectMemoryRegion(void* address, size_t size) { return true; }
        bool MemoryProtection::IsValidMemoryAddress(const void* ptr) { return ptr != nullptr; }
        bool MemoryProtection::IsExecutableMemory(const void* ptr) { return false; }
        
        bool AntiAnalysis::IsEmulator() { return false; }
        bool AntiAnalysis::IsSandbox() { return false; }
        bool AntiAnalysis::IsBeingAnalyzed() { return IsDebuggerPresent() || IsVirtualMachine(); }
        bool AntiAnalysis::IsMemoryDumped() { return false; }
        void AntiAnalysis::ObfuscateExecutionFlow() {}
        void AntiAnalysis::InsertRedHerrings() {}
        bool AntiAnalysis::DetectTimingAttack() { return false; }
        uint64_t AntiAnalysis::GetAccurateTimestamp() { 
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        }
        bool AntiAnalysis::IsExecutionTooSlow() { return false; }
        
        uint32_t IntegrityChecker::CalculateCodeHash(const void* code, size_t size) { return 0; }
        bool IntegrityChecker::VerifyCodeIntegrity(const void* code, size_t size, uint32_t expected_hash) { return true; }
        bool IntegrityChecker::VerifyStackIntegrity() { return true; }
        bool IntegrityChecker::VerifyHeapIntegrity() { return true; }
        void IntegrityChecker::InstallIntegrityChecks() {}
        void IntegrityChecker::PerformRuntimeIntegrityCheck() {}
        
        void ResourceMonitor::SetMemoryLimit(size_t max_bytes) { g_current_config.max_memory_allocation = max_bytes; }
        void ResourceMonitor::SetExecutionTimeLimit(uint32_t max_milliseconds) { g_current_config.max_execution_time = max_milliseconds; }
        void ResourceMonitor::SetRecursionLimit(uint32_t max_depth) { g_current_config.max_recursion_depth = max_depth; }
        size_t ResourceMonitor::GetCurrentMemoryUsage() { return g_memory_usage; }
        uint32_t ResourceMonitor::GetExecutionTime() { 
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now - g_start_time).count();
        }
        uint32_t ResourceMonitor::GetCurrentRecursionDepth() { return g_recursion_depth; }
        bool ResourceMonitor::IsMemoryLimitExceeded() { return g_memory_usage > g_current_config.max_memory_allocation; }
        bool ResourceMonitor::IsExecutionTimeLimitExceeded() { return GetExecutionTime() > g_current_config.max_execution_time; }
        bool ResourceMonitor::IsRecursionLimitExceeded() { return g_recursion_depth > g_current_config.max_recursion_depth; }
        void ResourceMonitor::ResetCounters() { g_memory_usage = 0; g_recursion_depth = 0; g_start_time = std::chrono::steady_clock::now(); }
        
        void SecurityLogger::ClearLog() { 
            std::lock_guard<std::mutex> lock(g_security_mutex);
            g_security_events.clear(); 
        }
        void SecurityLogger::SetLogLevel(int level) {}
        
        bool SecurityHardening::IsSecureEnvironment() { return VerifyEnvironment(); }
        bool SecurityHardening::TestBufferOverflow() { return true; }
        bool SecurityHardening::TestIntegerOverflow() { return true; }
        bool SecurityHardening::TestMemoryCorruption() { return true; }
        bool SecurityHardening::TestInjectionAttacks() { return true; }
        bool SecurityHardening::DisableDebugging() { return true; }
        bool SecurityHardening::InstallHooks() { return true; }
        void SecurityHardening::Shutdown() { m_initialized = false; }
        
        void* SecureMemmove(void* dest, const void* src, size_t size) { return std::memmove(dest, src, size); }
        int SecureMemcmp(const void* ptr1, const void* ptr2, size_t size) { return std::memcmp(ptr1, ptr2, size); }
        size_t SecureStrlen(const char* str, size_t max_len) { return strnlen(str, max_len); }
        char* SecureStrcpy(char* dest, const char* src, size_t dest_size) { return strncpy(dest, src, dest_size - 1); }
        char* SecureStrcat(char* dest, const char* src, size_t dest_size) { return strncat(dest, src, dest_size - strlen(dest) - 1); }
        bool IsValidUTF8(const char* str, size_t len) { return true; }

    } // namespace Security
} // namespace AetherVisor