#include "EventManager.h"
#include <stdexcept>

// Size of a 32-bit relative JMP instruction
constexpr size_t JMP_SIZE = 5;

namespace AetherVisor {
    namespace Payload {

        EventManager& EventManager::GetInstance() {
            static EventManager instance;
            return instance;
        }

        EventManager::~EventManager() {
            // Create a copy of keys to avoid iterator invalidation issues.
            std::vector<void*> targets;
            for (const auto& pair : m_hooks) {
                targets.push_back(pair.first);
            }
            for (void* target : targets) {
                Uninstall(target);
            }
        }

        bool EventManager::Install(void* targetFunc, void* detourFunc) {
            if (!targetFunc || !detourFunc || m_hooks.count(targetFunc)) {
                return false;
            }

            HookInfo info;
            info.targetFunc = targetFunc;
            info.detourFunc = detourFunc;
            info.originalBytes.resize(JMP_SIZE);

            // 1. Save original bytes.
            memcpy(info.originalBytes.data(), targetFunc, JMP_SIZE);

            // 2. Allocate memory for the trampoline.
            info.trampolineFunc = VirtualAlloc(nullptr, JMP_SIZE + JMP_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!info.trampolineFunc) {
                return false;
            }

            // 3. Write the original bytes into the trampoline.
            memcpy(info.trampolineFunc, info.originalBytes.data(), JMP_SIZE);

            // 4. Write a JMP from the trampoline back to the original function (after the hook).
            BYTE* trampoline_jmp_addr = (BYTE*)info.trampolineFunc + JMP_SIZE;
            DWORD_PTR relative_jmp = ((BYTE*)targetFunc + JMP_SIZE) - (trampoline_jmp_addr + JMP_SIZE);
            *(trampoline_jmp_addr) = 0xE9; // JMP opcode
            *(DWORD_PTR*)(trampoline_jmp_addr + 1) = relative_jmp;

            // 5. Write the JMP from the target function to our detour.
            DWORD oldProtect;
            VirtualProtect(targetFunc, JMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);

            BYTE* target_jmp_addr = (BYTE*)targetFunc;
            relative_jmp = ((BYTE*)detourFunc) - (target_jmp_addr + JMP_SIZE);
            *(target_jmp_addr) = 0xE9; // JMP opcode
            *(DWORD_PTR*)(target_jmp_addr + 1) = relative_jmp;

            VirtualProtect(targetFunc, JMP_SIZE, oldProtect, &oldProtect);

            m_hooks[targetFunc] = info;
            return true;
        }

        bool EventManager::Uninstall(void* targetFunc) {
            if (!m_hooks.count(targetFunc)) {
                return false;
            }

            auto& info = m_hooks.at(targetFunc);

            // Restore the original bytes to the function.
            DWORD oldProtect;
            VirtualProtect(targetFunc, info.originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
            memcpy(targetFunc, info.originalBytes.data(), info.originalBytes.size());
            VirtualProtect(targetFunc, info.originalBytes.size(), oldProtect, &oldProtect);

            // Free the trampoline memory.
            VirtualFree(info.trampolineFunc, 0, MEM_RELEASE);

            m_hooks.erase(targetFunc);
            return true;
        }

        template<typename T>
        T EventManager::GetOriginal(void* targetFunc) {
            auto it = m_hooks.find(targetFunc);
            if (it != m_hooks.end()) {
                return reinterpret_cast<T>(it->second.trampolineFunc);
            }
            return nullptr;
        }

        // Explicit template instantiations to avoid linker errors if GetOriginal is used in other cpp files.
        template send_t EventManager::GetOriginal<send_t>(void* target);
        template recv_t EventManager::GetOriginal<recv_t>(void* target);

    } // namespace Payload
} // namespace AetherVisor
