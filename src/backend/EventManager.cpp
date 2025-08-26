#include "EventManager.h"
#include <stdexcept>

// A simple implementation of a 5-byte relative jump (for x86).
// This is a placeholder for a more robust multi-architecture solution.
void WriteDetour(void* target, void* detour) {
    DWORD oldProtect;
    VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

    BYTE jmp_instruction[5];
    jmp_instruction[0] = 0xE9; // JMP opcode
    // Calculate relative address: target -> detour
    *(DWORD*)(jmp_instruction + 1) = (DWORD)((DWORD_PTR)detour - (DWORD_PTR)target - 5);

    memcpy(target, jmp_instruction, 5);

    VirtualProtect(target, 5, oldProtect, &oldProtect);
}

namespace AetherVisor {
    namespace Payload {

        EventManager& EventManager::GetInstance() {
            static EventManager instance;
            return instance;
        }

        EventManager::~EventManager() {
            // Uninstall all hooks on destruction to prevent crashes.
            for (auto const& [target, hook] : m_hooks) {
                // This is a simplified cleanup. A real implementation would need to be
                // more careful, especially with multi-threading.
                Uninstall(hook.targetFunc);
            }
            m_hooks.clear();
        }

        bool EventManager::Install(void* targetFunc, void* detourFunc) {
            if (!targetFunc || !detourFunc) {
                return false;
            }
            if (m_hooks.count(targetFunc)) {
                // Hook already exists, do not install again.
                return false;
            }

            HookInfo info;
            info.targetFunc = targetFunc;
            info.detourFunc = detourFunc;
            info.originalBytes.resize(5); // Assuming a 5-byte hook for now.

            // Save the original bytes before we overwrite them.
            memcpy(info.originalBytes.data(), targetFunc, 5);

            // Write the JMP instruction to the target function.
            WriteDetour(targetFunc, detourFunc);

            // Store the hook information.
            m_hooks[targetFunc] = info;

            return true;
        }

        bool EventManager::Uninstall(void* targetFunc) {
            if (!targetFunc || !m_hooks.count(targetFunc)) {
                return false;
            }

            auto& info = m_hooks.at(targetFunc);

            DWORD oldProtect;
            VirtualProtect(targetFunc, info.originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);

            // Restore the original bytes to the function.
            memcpy(targetFunc, info.originalBytes.data(), info.originalBytes.size());

            VirtualProtect(targetFunc, info.originalBytes.size(), oldProtect, &oldProtect);

            // Remove the hook from our map.
            m_hooks.erase(targetFunc);

            return true;
        }

    } // namespace Payload
} // namespace AetherVisor
