#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "SecurityAudit.h"
#include <vector>
#include <mutex>

namespace AetherVisor::Security {
    struct SecurityAudit::Impl {
        std::vector<SecurityEvent> events;
        mutable std::mutex events_mutex;
        bool system_compromised = false;
    };
    
    SecurityAudit::SecurityAudit() : pImpl(std::make_unique<Impl>()) {}
    SecurityAudit::~SecurityAudit() = default;
    
    std::vector<SecurityEvent> SecurityAudit::GetSecurityEvents() const {
        std::lock_guard<std::mutex> lock(pImpl->events_mutex);
        return pImpl->events;
    }
    
    void SecurityAudit::LogSecurityEvent(const SecurityEvent& event) {
        std::lock_guard<std::mutex> lock(pImpl->events_mutex);
        pImpl->events.push_back(event);
        if (pImpl->events.size() > 1000) {
            pImpl->events.erase(pImpl->events.begin(), pImpl->events.begin() + 500);
        }
    }
    
    void SecurityAudit::ClearAuditLog() {
        std::lock_guard<std::mutex> lock(pImpl->events_mutex);
        pImpl->events.clear();
    }
    
    bool SecurityAudit::IsSystemCompromised() const {
        std::lock_guard<std::mutex> lock(pImpl->events_mutex);
        return pImpl->system_compromised;
    }
}
