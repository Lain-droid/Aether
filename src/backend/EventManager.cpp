#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "EventManager.h"
#include <stdexcept>
#include <cstring>
#ifdef _WIN32
#include <Windows.h>
#endif

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
            #ifdef _WIN32
            info.trampolineFunc = VirtualAlloc(nullptr, JMP_SIZE + JMP_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            #else
            info.trampolineFunc = std::malloc(JMP_SIZE + JMP_SIZE);
            #endif
            if (!info.trampolineFunc) {
                return false;
            }

            // 3. Write the original bytes into the trampoline.
            memcpy(info.trampolineFunc, info.originalBytes.data(), JMP_SIZE);

            // 4. Write a JMP from the trampoline back to the original function (after the hook).
            std::uint8_t* trampoline_jmp_addr = (std::uint8_t*)info.trampolineFunc + JMP_SIZE;
            std::uintptr_t relative_jmp = reinterpret_cast<std::uintptr_t>((std::uint8_t*)targetFunc + JMP_SIZE) - (reinterpret_cast<std::uintptr_t>(trampoline_jmp_addr) + JMP_SIZE);
            *(trampoline_jmp_addr) = 0xE9; // JMP opcode
            *(std::uint32_t*)(trampoline_jmp_addr + 1) = static_cast<std::uint32_t>(relative_jmp);

            // 5. Write the JMP from the target function to our detour.
            #ifdef _WIN32
            DWORD oldProtect;
            VirtualProtect(targetFunc, JMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);
            #endif

            std::uint8_t* target_jmp_addr = (std::uint8_t*)targetFunc;
            relative_jmp = reinterpret_cast<std::uintptr_t>((std::uint8_t*)detourFunc) - (reinterpret_cast<std::uintptr_t>(target_jmp_addr) + JMP_SIZE);
            *(target_jmp_addr) = 0xE9; // JMP opcode
            *(std::uint32_t*)(target_jmp_addr + 1) = static_cast<std::uint32_t>(relative_jmp);

            #ifdef _WIN32
            VirtualProtect(targetFunc, JMP_SIZE, oldProtect, &oldProtect);
            #endif

            m_hooks[targetFunc] = info;
            return true;
        }

        bool EventManager::Uninstall(void* targetFunc) {
            if (!m_hooks.count(targetFunc)) {
                return false;
            }

            auto& info = m_hooks.at(targetFunc);

            // Restore the original bytes to the function.
            #ifdef _WIN32
            DWORD oldProtect;
            VirtualProtect(targetFunc, info.originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
            #endif
            std::memcpy(targetFunc, info.originalBytes.data(), info.originalBytes.size());
            #ifdef _WIN32
            VirtualProtect(targetFunc, info.originalBytes.size(), oldProtect, &oldProtect);
            #endif

            // Free the trampoline memory.
            #ifdef _WIN32
            VirtualFree(info.trampolineFunc, 0, MEM_RELEASE);
            #else
            std::free(info.trampolineFunc);
            #endif

            m_hooks.erase(targetFunc);
            return true;
        }

        // Note: explicit template instantiations removed for portability.

    } // namespace Payload
} // namespace AetherVisor
