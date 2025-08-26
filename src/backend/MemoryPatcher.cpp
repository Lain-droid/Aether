#include "MemoryPatcher.h"

namespace AetherVisor {
    namespace Payload {

        MemoryPatcher& MemoryPatcher::GetInstance() {
            static MemoryPatcher instance;
            return instance;
        }

        MemoryPatcher::~MemoryPatcher() {
            RevertAllPatches();
        }

        bool MemoryPatcher::ApplyPatchConditionally(void* targetAddress, const std::vector<BYTE>& patchData, Backend::RiskLevel requiredLevel) {
            if (!targetAddress || patchData.empty()) {
                return false;
            }

            // Check with the AI controller before performing this high-risk action.
            if (!Backend::AIController::GetInstance().ShouldPerformAction(requiredLevel)) {
                return false; // Risk level is too high.
            }

            if (m_patches.count(targetAddress)) {
                // A patch already exists at this address. Revert it first.
                RevertPatch(targetAddress);
            }

            PatchInfo info;
            info.targetAddress = targetAddress;
            info.patchBytes = patchData;
            info.originalBytes.resize(patchData.size());

            // Read and save the original bytes before patching.
            memcpy(info.originalBytes.data(), targetAddress, patchData.size());

            // Apply the new patch.
            if (WriteMemory(targetAddress, patchData)) {
                info.isActive = true;
                m_patches[targetAddress] = info;

                // Report this event to the AI controller.
                Backend::AIController::GetInstance().ReportEvent(Backend::AIEventType::MEMORY_PATCH_APPLIED);
                return true;
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

        bool MemoryPatcher::WriteMemory(void* address, const std::vector<BYTE>& data) {
            if (!address || data.empty()) return false;

            DWORD oldProtect;
            if (VirtualProtect(address, data.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy(address, data.data(), data.size());
                VirtualProtect(address, data.size(), oldProtect, &oldProtect);
                return true;
            }
            return false;
        }

    } // namespace Payload
} // namespace AetherVisor
