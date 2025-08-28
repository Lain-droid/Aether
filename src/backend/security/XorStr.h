#pragma once

#include <string>
#include <array>
#include <utility>

// This is a common C++ technique for compile-time string obfuscation.
// The string is encrypted with a random key at compile time, and only
// decrypted in-place when it's accessed, leaving no plaintext in memory
// or in the binary file.

namespace AetherVisor {
    namespace Security {

        template <typename T, T... S, typename K, K... S_KEY>
        constexpr std::array<T, sizeof...(S)> xor_cypher(
            const std::array<T, sizeof...(S)>& str,
            std::integer_sequence<T, S...>,
            std::integer_sequence<K, S_KEY...>) {
            return { static_cast<T>(str[S] ^ static_cast<T>(std::array<K, sizeof...(S_KEY)>{S_KEY...}[S % sizeof...(S_KEY)]))... };
        }

        template <typename T, std::size_t N, std::size_t K>
        class XorStr {
        public:
            constexpr XorStr(const T(&str)[N], const T(&key)[K])
                : m_encrypted(xor_cypher(
                    [&]() { std::array<T, N> arr{}; for (size_t i = 0; i < N; ++i) arr[i] = str[i]; return arr; }(),
                    std::make_integer_sequence<T, N>(),
                    std::make_integer_sequence<T, K>()))
                , key(key) {}

            const T* get() const {
                for (size_t i = 0; i < N; ++i) {
                    m_decrypted[i] = m_encrypted[i] ^ key[i % K];
                }
                return m_decrypted.data();
            }

        private:
            mutable std::array<T, N> m_decrypted{};
            const std::array<T, N> m_encrypted;
            const T(&key)[K];
        };

// Helper macro to make usage cleaner.
#define XorS(str) (AetherVisor::Security::XorStr(str, __TIME__).get())

    } // namespace Security
} // namespace AetherVisor
