#include "MemoryPatcher.h"
#include <cstring>
#ifdef _WIN32
#include <Windows.h>
#endif
#ifndef _WIN32
#include <sys/mman.h>
#endif

namespace AetherVisor {
    namespace Payload {

        MemoryPatcher& MemoryPatcher::GetInstance() {
            static MemoryPatcher instance;
            return instance;
        }

        MemoryPatcher::~MemoryPatcher() {
            RevertAllPatches();
        }

        bool MemoryPatcher::ApplyPatchConditionally(void* targetAddress, const std::vector<ByteType>& patchData, Backend::RiskLevel requiredLevel) {
            if (!targetAddress || patchData.empty()) {
                return false;
            }

            // Validate patch size to prevent buffer overflow
            if (patchData.size() > 4096) { // Reasonable limit for patches
                return false;
            }

            // Check with the AI controller before performing this high-risk action.
            if (!Backend::AIController::GetInstance().ShouldPerformAction(requiredLevel)) {
                return false; // Risk level is too high.
            }

            // Verify target memory is valid and accessible
            #ifdef _WIN32
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(targetAddress, &mbi, sizeof(mbi)) == 0 ||
                mbi.State != MEM_COMMIT ||
                mbi.RegionSize < patchData.size()) {
                return false;
            }
            #endif

            if (m_patches.count(targetAddress)) {
                // A patch already exists at this address. Revert it first.
                RevertPatch(targetAddress);
            }

            PatchInfo info;
            info.targetAddress = targetAddress;
            info.patchBytes = patchData;
            info.originalBytes.resize(patchData.size());

            try {
                // Read and save the original bytes before patching with bounds checking.
                std::memcpy(info.originalBytes.data(), targetAddress, patchData.size());

                // Apply the new patch.
                if (WriteMemory(targetAddress, patchData)) {
                    info.isActive = true;
                    m_patches[targetAddress] = info;

                    // Report this event to the AI controller.
                    Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::MEMORY_PATCH_APPLIED);
                    return true;
                }
            } catch (const std::exception&) {
                // Failed to patch - don't leave partial state
                return false;
            }

            return false;
        }

        bool MemoryPatcher::RevertPatch(void* targetAddress) {
            if (!m_patches.count(targetAddress)) {
                return false;
            }

            auto& info = m_patches.at(targetAddress);
            if (!info.isActive) {
                return false; // Patch is already inactive.
            }

            if (WriteMemory(targetAddress, info.originalBytes)) {
                m_patches.erase(targetAddress);
                return true;
            }

            return false;
        }

        void MemoryPatcher::RevertAllPatches() {
            // Iterate over a copy of the keys to avoid issues with erasing while iterating.
            std::vector<void*> addresses;
            for (const auto& pair : m_patches) {
                addresses.push_back(pair.first);
            }

            for (void* address : addresses) {
                RevertPatch(address);
            }
        }

        bool MemoryPatcher::WriteMemory(void* address, const std::vector<ByteType>& data) {
            if (!address || data.empty()) return false;

            // Additional bounds checking
            if (data.size() > 16384) { // 16KB limit for safety
                return false;
            }

            #ifdef _WIN32
            // Verify memory region before writing
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(address, &mbi, sizeof(mbi)) == 0 ||
                mbi.State != MEM_COMMIT ||
                mbi.RegionSize < data.size()) {
                return false;
            }

            DWORD oldProtect;
            if (VirtualProtect(address, data.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                // Use secure memory copy
                std::memcpy(address, data.data(), data.size());
                
                // Restore original protection immediately
                if (!VirtualProtect(address, data.size(), oldProtect, &oldProtect)) {
                    // Log security warning but don't fail - memory was written
                    Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::SUSPICIOUS_API_CALL);
                }
                return true;
            }
            return false;
            #else
            // Non-Windows: verify page permissions before writing
            #if defined(__unix__) || defined(__APPLE__)
            // Use mprotect to temporarily make writable if needed
            if (mprotect(address, data.size(), PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
                return false;
            }
            #endif
            
            std::memcpy(address, data.data(), data.size());
            
            #if defined(__unix__) || defined(__APPLE__)
            // Restore original protection (simplified)
            mprotect(address, data.size(), PROT_READ | PROT_EXEC);
            #endif
            
            return true;
            #endif
        }

    } // namespace Payload
} // namespace AetherVisor
