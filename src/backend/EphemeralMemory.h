#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
using ByteType = BYTE;
#else
using ByteType = std::uint8_t;
#endif

namespace AetherVisor {
    namespace Payload {

        /**
         * @class EphemeralMemory
         * @brief Manages a temporary, self-destructing block of memory.
         *
         * This class encapsulates the concept of "Ephemeral Payloads" or
         * "Ephemeral Sandbox Objects". An instance of this class represents a
         * block of memory that is securely zeroed-out upon destruction,
         * minimizing the forensic footprint.
         */
        class EphemeralMemory {
        public:
            /**
             * @brief Allocates a block of ephemeral memory.
             * @param size The number of bytes to allocate.
             */
            explicit EphemeralMemory(size_t size);

            /**
             * @brief Destructor that securely erases and frees the memory.
             */
            ~EphemeralMemory();

            /**
             * @brief Writes data to the allocated memory block.
             * @param data The bytes to write.
             * @return True if the write was successful.
             */
            bool Write(const std::vector<ByteType>& data);

            /**
             * @brief Reads data from the allocated memory block.
             * @param size The number of bytes to read.
             * @return A vector of bytes containing the data. An empty vector indicates failure.
             */
            std::vector<ByteType> Read(size_t size) const;

            /**
             * @brief Gets the base address of the allocated memory.
             * @return A pointer to the base address, or nullptr if not allocated.
             */
            void* GetAddress() const;

            /**
             * @brief Gets the size of the allocated memory.
             * @return The size in bytes.
             */
            size_t GetSize() const;

            // Disable copy and assignment to prevent mishandling of the raw pointer.
            EphemeralMemory(const EphemeralMemory&) = delete;
            EphemeralMemory& operator=(const EphemeralMemory&) = delete;

            // Enable move semantics to allow transferring ownership.
            EphemeralMemory(EphemeralMemory&& other) noexcept;
            EphemeralMemory& operator=(EphemeralMemory&& other) noexcept;

        private:
            void* m_address = nullptr;
            size_t m_size = 0;
        };

    } // namespace Payload
} // namespace AetherVisor
