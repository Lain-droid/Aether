#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <windows.h>

namespace Aether::Security {

enum class SecurityEventType {
    ProcessCreation,
    ProcessTermination,
    ProcessInjection,
    FileModification,
    RegistryModification,
    NetworkConnection,
    SuspiciousProcess,
    SystemModification,
    PrivilegeEscalation,
    Unauthorized Access
};

enum class SecuritySeverity {
    Low = 0,
    Medium = 1,
    High = 2,
    Critical = 3
};

enum AuditAction {
    Log = 1,
    Alert = 2,
    Block = 4,
    Quarantine = 8
};

struct SecurityEvent {
    SecurityEventType type;
    SecuritySeverity severity;
    std::chrono::system_clock::time_point timestamp;
    std::string description;
    std::unordered_map<std::string, std::string> details;
};

struct AuditRule {
    std::string name;
    SecurityEventType eventType;
    SecuritySeverity severity;
    int action; // Bitfield of AuditAction
    std::string condition; // Optional condition string
};

struct SystemSecurityAssessment {
    bool antivirusEnabled = false;
    bool firewallEnabled = false;
    bool systemUpToDate = false;
    bool uacEnabled = false;
    bool rootkitDetected = false;
    std::string osVersion;
    std::vector<std::string> installedSecurity;
};

struct ProcessSecurityAssessment {
    int totalProcesses = 0;
    int elevatedProcesses = 0;
    int systemProcesses = 0;
    int userProcesses = 0;
    int unsignedProcesses = 0;
    int suspiciousProcesses = 0;
    std::vector<std::string> criticalProcesses;
};

struct NetworkSecurityAssessment {
    std::vector<int> openPorts;
    std::vector<std::string> suspiciousConnections;
    bool secureConfiguration = true;
    std::vector<std::string> activeConnections;
};

struct FilesystemSecurityAssessment {
    bool insecurePermissions = false;
    bool systemIntegrityOk = true;
    std::vector<std::string> suspiciousFiles;
    std::vector<std::string> modifiedSystemFiles;
};

struct SecurityReport {
    std::chrono::system_clock::time_point timestamp;
    int overallScore = 0;
    SystemSecurityAssessment systemSecurity;
    ProcessSecurityAssessment processSecurity;
    NetworkSecurityAssessment networkSecurity;
    FilesystemSecurityAssessment filesystemSecurity;
    std::vector<SecurityEvent> recentEvents;
    std::vector<std::string> recommendations;
};

/**
 * @brief Security Audit and Monitoring System
 * 
 * Provides comprehensive security monitoring and auditing capabilities
 * for the Aether application and the host system.
 * 
 * Features:
 * - Real-time process monitoring
 * - File system integrity checking
 * - Registry monitoring
 * - Network activity monitoring
 * - Security event logging
 * - Automated security assessment
 * - Custom audit rules
 */
class SecurityAudit {
public:
    SecurityAudit();
    ~SecurityAudit();

    /**
     * @brief Initialize the security audit system
     * @return true if initialization successful
     */
    bool Initialize();

    /**
     * @brief Start real-time security monitoring
     * @return true if monitoring started successfully
     */
    bool StartMonitoring();

    /**
     * @brief Stop security monitoring
     */
    void StopMonitoring();

    /**
     * @brief Generate comprehensive security report
     * @return Security assessment report
     */
    SecurityReport GenerateReport();

    /**
     * @brief Add a custom audit rule
     * @param rule The audit rule to add
     */
    void AddAuditRule(const AuditRule& rule);

    /**
     * @brief Remove an audit rule by name
     * @param ruleName Name of the rule to remove
     */
    void RemoveAuditRule(const std::string& ruleName);

    /**
     * @brief Get recent security events
     * @param maxEvents Maximum number of events to return (0 = all)
     * @return Vector of security events
     */
    std::vector<SecurityEvent> GetEvents(size_t maxEvents = 0);

    /**
     * @brief Get security events by type
     * @param type Event type to filter by
     * @param maxEvents Maximum number of events to return
     * @return Vector of filtered security events
     */
    std::vector<SecurityEvent> GetEventsByType(SecurityEventType type, size_t maxEvents = 0);

    /**
     * @brief Log a security event
     * @param event The security event to log
     */
    void LogSecurityEvent(const SecurityEvent& event);

    /**
     * @brief Check if monitoring is currently active
     * @return true if monitoring is active
     */
    bool IsMonitoring() const;

    /**
     * @brief Set the maximum number of events to keep in memory
     * @param maxSize Maximum event log size
     */
    void SetMaxEventLogSize(size_t maxSize);

    /**
     * @brief Enable or disable file logging
     * @param enable true to enable file logging
     * @param filePath Path to log file (optional)
     */
    void SetFileLogging(bool enable, const std::string& filePath = "");

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

    // Monitoring thread
    static DWORD WINAPI MonitoringThreadProc(LPVOID lpParam);
    DWORD MonitoringLoop();

    // Monitoring functions
    void MonitorProcesses();
    void MonitorFileSystem();
    void MonitorRegistry();
    void MonitorNetwork();

    // Security checks
    bool CheckProcessInjection(DWORD processId);

    // Assessment functions
    SystemSecurityAssessment AssessSystemSecurity();
    ProcessSecurityAssessment AssessProcessSecurity();
    NetworkSecurityAssessment AssessNetworkSecurity();
    FilesystemSecurityAssessment AssessFilesystemSecurity();

    // Report generation
    std::vector<SecurityEvent> GetRecentEvents(size_t maxEvents);
    int CalculateSecurityScore(const SecurityReport& report);
    std::vector<std::string> GenerateRecommendations(const SecurityReport& report);

    // Configuration
    void LoadAuditRules();
    void LoadTrustedProcesses();
    void SetupDefaultRules();

    // Event processing
    void ProcessAuditRules(const SecurityEvent& event);
    void LogEventToFile(const SecurityEvent& event);

    // System assessment helpers
    bool CheckAntivirusStatus();
    bool CheckFirewallStatus();
    bool CheckSystemUpdates();
    bool CheckUACStatus();
    bool CheckForRootkits();

    void CountProcessPrivileges(ProcessSecurityAssessment& assessment);
    int CountUnsignedProcesses();
    int CountSuspiciousProcesses();

    std::vector<int> GetOpenPorts();
    std::vector<std::string> GetSuspiciousConnections();
    bool CheckNetworkConfiguration();

    bool CheckFilePermissions();
    std::vector<std::string> FindSuspiciousFiles();
    bool CheckSystemFileIntegrity();
};

} // namespace Aether::Security