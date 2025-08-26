#include "EphemeralMemory.h"
#include "security/XorStr.h"
#include <stdexcept>

namespace AetherVisor {
    namespace Payload {

        EphemeralMemory::EphemeralMemory(size_t size) : m_size(size) {
            if (size == 0) {
                throw std::invalid_argument(XorS("Allocation size cannot be zero."));
            }
            m_address = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!m_address) {
                throw std::runtime_error(XorS("Failed to allocate ephemeral memory."));
            }
        }

        EphemeralMemory::~EphemeralMemory() {
            if (m_address) {
                // Securely wipe the memory before freeing it.
                SecureZeroMemory(m_address, m_size);
                VirtualFree(m_address, 0, MEM_RELEASE);
            }
        }

        bool EphemeralMemory::Write(const std::vector<BYTE>& data) {
            if (!m_address || data.size() > m_size) {
                return false;
            }
            memcpy(m_address, data.data(), data.size());
            return true;
        }

        std::vector<BYTE> EphemeralMemory::Read(size_t size) const {
            if (!m_address || size > m_size) {
                return {}; // Return empty vector on failure
            }
            std::vector<BYTE> buffer(size);
            memcpy(buffer.data(), m_address, size);
            return buffer;
        }

        void* EphemeralMemory::GetAddress() const {
            return m_address;
        }

        size_t EphemeralMemory::GetSize() const {
            return m_size;
        }

        // Move constructor
        EphemeralMemory::EphemeralMemory(EphemeralMemory&& other) noexcept
            : m_address(other.m_address), m_size(other.m_size) {
            // Leave the moved-from object in a valid but empty state.
            other.m_address = nullptr;
            other.m_size = 0;
        }

        // Move assignment operator
        EphemeralMemory& EphemeralMemory::operator=(EphemeralMemory&& other) noexcept {
            if (this != &other) {
                // Clean up existing resource first.
                if (m_address) {
                    SecureZeroMemory(m_address, m_size);
                    VirtualFree(m_address, 0, MEM_RELEASE);
                }

                // Pilfer the resources from the other object.
                m_address = other.m_address;
                m_size = other.m_size;

                // Leave the moved-from object in a valid but empty state.
                other.m_address = nullptr;
                other.m_size = 0;
            }
            return *this;
        }

    } // namespace Payload
} // namespace AetherVisor
