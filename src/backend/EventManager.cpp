#include "EventManager.h"
#include <stdexcept>
#include <cstring>
#include <limits>
#ifdef _WIN32
#include <Windows.h>
#endif
#ifndef _WIN32
#include <sys/mman.h>
#include <cstdlib>
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

            // 1. Save original bytes with bounds checking.
            if (!targetFunc || !detourFunc) {
                return false;
            }
            
            // Verify target function is valid memory
            #ifdef _WIN32
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(targetFunc, &mbi, sizeof(mbi)) == 0 ||
                mbi.State != MEM_COMMIT || 
                !(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) {
                return false;
            }
            #endif
            
            // Use secure memory copy with bounds checking
            if (info.originalBytes.size() < JMP_SIZE) {
                info.originalBytes.resize(JMP_SIZE);
            }
            std::memcpy(info.originalBytes.data(), targetFunc, JMP_SIZE);

            // 2. Allocate memory for the trampoline with proper permissions.
            size_t trampolineSize = JMP_SIZE + JMP_SIZE;
            #ifdef _WIN32
            info.trampolineFunc = VirtualAlloc(nullptr, trampolineSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            #else
            info.trampolineFunc = std::aligned_alloc(16, trampolineSize); // Aligned allocation
            if (info.trampolineFunc) {
                // Set execute permissions on non-Windows
                #if defined(__unix__) || defined(__APPLE__)
                mprotect(info.trampolineFunc, trampolineSize, PROT_READ | PROT_WRITE | PROT_EXEC);
                #endif
            }
            #endif
            if (!info.trampolineFunc) {
                return false;
            }

            // 3. Write the original bytes into the trampoline with bounds checking.
            std::memcpy(info.trampolineFunc, info.originalBytes.data(), JMP_SIZE);

            // 4. Write a JMP from the trampoline back to the original function (after the hook).
            std::uint8_t* trampoline_jmp_addr = static_cast<std::uint8_t*>(info.trampolineFunc) + JMP_SIZE;
            
            // Verify pointer arithmetic doesn't overflow
            if (trampoline_jmp_addr < static_cast<std::uint8_t*>(info.trampolineFunc) ||
                trampoline_jmp_addr >= static_cast<std::uint8_t*>(info.trampolineFunc) + trampolineSize) {
                #ifdef _WIN32
                VirtualFree(info.trampolineFunc, 0, MEM_RELEASE);
                #else
                std::free(info.trampolineFunc);
                #endif
                return false;
            }
            
            // Calculate relative jump with overflow checking
            std::uintptr_t target_addr = reinterpret_cast<std::uintptr_t>(static_cast<std::uint8_t*>(targetFunc) + JMP_SIZE);
            std::uintptr_t trampoline_addr = reinterpret_cast<std::uintptr_t>(trampoline_jmp_addr) + JMP_SIZE;
            
            if (target_addr < trampoline_addr) {
                // Check for underflow
                std::uintptr_t diff = trampoline_addr - target_addr;
                if (diff > static_cast<std::uintptr_t>(std::numeric_limits<std::int32_t>::max())) {
                    #ifdef _WIN32
                    VirtualFree(info.trampolineFunc, 0, MEM_RELEASE);
                    #else
                    std::free(info.trampolineFunc);
                    #endif
                    return false;
                }
            } else {
                // Check for overflow
                std::uintptr_t diff = target_addr - trampoline_addr;
                if (diff > static_cast<std::uintptr_t>(std::numeric_limits<std::int32_t>::max())) {
                    #ifdef _WIN32
                    VirtualFree(info.trampolineFunc, 0, MEM_RELEASE);
                    #else
                    std::free(info.trampolineFunc);
                    #endif
                    return false;
                }
            }
            
            std::int32_t relative_jmp = static_cast<std::int32_t>(target_addr - trampoline_addr);
            *trampoline_jmp_addr = 0xE9; // JMP opcode
            *reinterpret_cast<std::int32_t*>(trampoline_jmp_addr + 1) = relative_jmp;

            // 5. Write the JMP from the target function to our detour with bounds checking.
            #ifdef _WIN32
            DWORD oldProtect;
            if (!VirtualProtect(targetFunc, JMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                VirtualFree(info.trampolineFunc, 0, MEM_RELEASE);
                return false;
            }
            #endif

            std::uint8_t* target_jmp_addr = static_cast<std::uint8_t*>(targetFunc);
            
            // Calculate relative jump to detour with overflow checking
            std::uintptr_t detour_addr = reinterpret_cast<std::uintptr_t>(detourFunc);
            std::uintptr_t target_call_addr = reinterpret_cast<std::uintptr_t>(target_jmp_addr) + JMP_SIZE;
            
            if (detour_addr < target_call_addr) {
                std::uintptr_t diff = target_call_addr - detour_addr;
                if (diff > static_cast<std::uintptr_t>(std::numeric_limits<std::int32_t>::max())) {
                    #ifdef _WIN32
                    VirtualProtect(targetFunc, JMP_SIZE, oldProtect, &oldProtect);
                    VirtualFree(info.trampolineFunc, 0, MEM_RELEASE);
                    #else
                    std::free(info.trampolineFunc);
                    #endif
                    return false;
                }
            } else {
                std::uintptr_t diff = detour_addr - target_call_addr;
                if (diff > static_cast<std::uintptr_t>(std::numeric_limits<std::int32_t>::max())) {
                    #ifdef _WIN32
                    VirtualProtect(targetFunc, JMP_SIZE, oldProtect, &oldProtect);
                    VirtualFree(info.trampolineFunc, 0, MEM_RELEASE);
                    #else
                    std::free(info.trampolineFunc);
                    #endif
                    return false;
                }
            }
            
            relative_jmp = static_cast<std::int32_t>(detour_addr - target_call_addr);
            *target_jmp_addr = 0xE9; // JMP opcode
            *reinterpret_cast<std::int32_t*>(target_jmp_addr + 1) = relative_jmp;

            #ifdef _WIN32
            if (!VirtualProtect(targetFunc, JMP_SIZE, oldProtect, &oldProtect)) {
                // Attempt to restore original bytes on failure
                std::memcpy(targetFunc, info.originalBytes.data(), JMP_SIZE);
                VirtualFree(info.trampolineFunc, 0, MEM_RELEASE);
                return false;
            }
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
