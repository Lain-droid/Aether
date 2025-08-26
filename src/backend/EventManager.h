#pragma once

#include <Windows.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

namespace AetherVisor {
    namespace Payload {

        // Represents a single installed hook.
        struct HookInfo {
            void* targetFunc;      // The original function that is hooked.
            void* detourFunc;      // Our function that the target jumps to.
            std::vector<BYTE> originalBytes; // The original bytes overwritten at the target.
        };

        /**
         * @class EventManager
         * @brief A centralized manager for installing and uninstalling function hooks.
         *
         * This class provides a generic mechanism to detour functions at runtime.
         * It is designed to be the core of the "Event Layer", allowing for conditional
         * and ephemeral hooking based on application or AI logic.
         * This is a singleton to ensure all hooks are managed from one place.
         */
        class EventManager {
        public:
            // Gets the singleton instance of the EventManager.
            static EventManager& GetInstance();

            /**
             * @brief Installs a detour hook on a target function.
             * @param targetFunc Address of the function to hook.
             * @param detourFunc Address of the function to execute instead.
             * @return True if the hook was installed successfully, false otherwise.
             */
            bool Install(void* targetFunc, void* detourFunc);

            /**
             * @brief Uninstalls a hook from a target function.
             * @param targetFunc Address of the hooked function to restore.
             * @return True if the hook was removed successfully, false otherwise.
             */
            bool Uninstall(void* targetFunc);

            /**
             * @brief Retrieves the original function pointer for a hooked function.
             * This allows calling the original function from within the detour.
             * @tparam T The function pointer type.
             * @param targetFunc The address of the hooked function.
             * @return A callable function pointer to the original function.
             */
            template<typename T>
            T GetOriginal(void* targetFunc) {
                // In a real implementation, this would point to a "trampoline"
                // which executes the original bytes and then jumps back.
                auto it = m_hooks.find(targetFunc);
                if (it != m_hooks.end()) {
                    // This is a simplification. The trampoline logic is complex.
                    // For now, returning the original address is a placeholder.
                    return reinterpret_cast<T>(it->second.targetFunc);
                }
                return nullptr;
            }

        private:
            EventManager() = default;
            ~EventManager(); // Destructor should uninstall all hooks.

            // Private copy constructor and assignment operator to prevent cloning.
            EventManager(const EventManager&) = delete;
            EventManager& operator=(const EventManager&) = delete;

            // A map to store information about our active hooks.
            // Key: Pointer to the target function.
            // Value: HookInfo struct with details about the hook.
            std::unordered_map<void*, HookInfo> m_hooks;
        };

    } // namespace Payload
} // namespace AetherVisor
