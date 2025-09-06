#pragma once
#include "SecurityTypes.h"
#include <vector>
#include <string>

namespace AetherVisor::Security {
    class SignatureScanner {
    public:
        SignatureScanner();
        ~SignatureScanner() = default;
        
        void AddSignature(const std::vector<ByteType>& pattern, const std::string& name);
        bool ScanMemoryRegion(const void* start, size_t size) const;
        std::vector<std::string> GetDetectedSignatures() const;
        void ClearSignatures();
        
        // Static method for compatibility
        static void* FindPattern(void* moduleBase, const char* pattern);
        
    private:
        std::vector<std::vector<ByteType>> m_signatures;
        std::vector<std::string> m_signature_names;
        mutable std::vector<std::string> m_detected;
        
        bool MatchPattern(const ByteType* data, size_t data_size, 
                         const std::vector<ByteType>& pattern) const;
        static bool ParsePattern(const char* pattern, std::vector<ByteType>& outBytes, std::vector<bool>& outMask);
    };
}
