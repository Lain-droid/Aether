#pragma once

#include "AIController.h"
#include <cstdint>
#include <vector>
#include <unordered_map>

#ifdef _WIN32
#include <Windows.h>
using ByteType = BYTE;
#else
using ByteType = std::uint8_t;
using HANDLE = void*;
#endif

namespace AetherVisor {
    namespace Payload {

        // Holds the information needed to apply or revert a memory patch.
        struct PatchInfo {
            void* targetAddress;
            std::vector<ByteType> originalBytes;
            std::vector<ByteType> patchBytes;
            bool isActive = false;
        };

        /**
         * @class MemoryPatcher
         * @brief Manages the application and reversion of memory patches.
         *
         * This class integrates with the AIController to perform event-based
         * and conditional memory patching. It keeps track of all active patches
         * to ensure they can be reverted cleanly.
         */
        class MemoryPatcher {
        public:
            // Gets the singleton instance of the MemoryPatcher.
            static MemoryPatcher& GetInstance();

            /**
             * @brief Applies a memory patch if the current risk is acceptable.
             * @param targetAddress The memory address to patch.
             * @param patchData A vector of bytes representing the patch.
             * @param requiredLevel The maximum risk level at which this patch should be applied.
             * @return True if the patch was applied, false otherwise.
             */
            bool ApplyPatchConditionally(void* targetAddress, const std::vector<ByteType>& patchData, Backend::RiskLevel requiredLevel);

            /**
             * @brief Reverts a patch at a specific address.
             * @param targetAddress The address of the patch to revert.
             * @return True if the patch was reverted, false otherwise.
             */
            bool RevertPatch(void* targetAddress);

            /**
             * @brief Reverts all active patches.
             */
            void RevertAllPatches();

        private:
            MemoryPatcher() = default;
            ~MemoryPatcher(); // Should revert all patches on destruction.
            MemoryPatcher(const MemoryPatcher&) = delete;
            MemoryPatcher& operator=(const MemoryPatcher&) = delete;

            // Safely writes data to a process's memory.
            bool WriteMemory(void* address, const std::vector<ByteType>& data);

            // A map to store information about our active patches.
            // Key: Pointer to the target address.
            // Value: PatchInfo struct with details about the patch.
            std::unordered_map<void*, PatchInfo> m_patches;
        };

    } // namespace Payload
} // namespace AetherVisor
