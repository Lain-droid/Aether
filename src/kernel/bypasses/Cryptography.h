#pragma once

#include <ntifs.h>

namespace AetherVisor {
    namespace Bypasses {

        /*
         * This module provides the core infrastructure for all cryptographic and
         * obfuscation operations, fulfilling the "encrypt everything" requirement.
         */
        namespace Cryptography {

            // Encrypts a block of memory in-place using a dynamic key.
            NTSTATUS EncryptMemory(PVOID pData, SIZE_T size, ULONG64 key);

            // Decrypts a block of memory in-place.
            NTSTATUS DecryptMemory(PVOID pData, SIZE_T size, ULONG64 key);

            // Generates a polymorphic version of a given function, altering its
            // instruction sequence while preserving its behavior.
            NTSTATUS GeneratePolymorphicFunction(PVOID pFunction, PVOID* ppNewFunction);

            // Obfuscates critical strings at compile time or runtime.
            const char* ObfuscateString(const char* pString);

            // Verifies the integrity of a code block to detect tampering.
            BOOLEAN VerifyCodeIntegrity(PVOID pCode, SIZE_T size, ULONG64 expectedHash);

            // Initializes the cryptographic engine, e.g., seeding random number generators.
            NTSTATUS Initialize();
        }
    }
}
