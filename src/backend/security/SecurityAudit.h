#pragma once
#include "SecurityTypes.h"
#include <memory>

namespace AetherVisor::Security {
    class SecurityAudit {
    public:
        SecurityAudit();
        ~SecurityAudit();
        
        std::vector<SecurityEvent> GetSecurityEvents() const;
        void LogSecurityEvent(const SecurityEvent& event);
        void ClearAuditLog();
        bool IsSystemCompromised() const;
        
    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}
