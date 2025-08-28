#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace CryptoPP {
    class SecByteBlock;
}

namespace Aether::Security {

enum class EncryptionMode {
    AES_256_CBC,
    AES_256_GCM,
    RSA_OAEP
};

/**
 * @brief Advanced Encryption Manager
 * 
 * Provides comprehensive encryption services for the Aether application,
 * including symmetric and asymmetric encryption, secure hashing, and HMAC.
 * 
 * Features:
 * - AES-256 encryption (CBC and GCM modes)
 * - RSA encryption with OAEP padding
 * - PBKDF2 key derivation
 * - SHA-256 hashing
 * - HMAC generation and verification
 * - Secure random key generation
 */
class EncryptionManager {
public:
    EncryptionManager();
    ~EncryptionManager();

    /**
     * @brief Initialize the encryption manager
     * @return true if initialization successful
     */
    bool Initialize();

    /**
     * @brief Encrypt a string using the current encryption mode
     * @param plaintext The text to encrypt
     * @param password Password for key derivation (not used for RSA)
     * @return Base64-encoded encrypted string
     */
    std::string EncryptString(const std::string& plaintext, const std::string& password = "");

    /**
     * @brief Decrypt a string using the current encryption mode
     * @param ciphertext Base64-encoded encrypted string
     * @param password Password for key derivation (not used for RSA)
     * @return Decrypted plaintext
     */
    std::string DecryptString(const std::string& ciphertext, const std::string& password = "");

    /**
     * @brief Encrypt binary data
     * @param data The data to encrypt
     * @param password Password for key derivation
     * @return Encrypted data (salt + IV + ciphertext)
     */
    std::vector<uint8_t> EncryptData(const std::vector<uint8_t>& data, const std::string& password);

    /**
     * @brief Decrypt binary data
     * @param encryptedData Encrypted data (salt + IV + ciphertext)
     * @param password Password for key derivation
     * @return Decrypted data
     */
    std::vector<uint8_t> DecryptData(const std::vector<uint8_t>& encryptedData, const std::string& password);

    /**
     * @brief Generate SHA-256 hash of input
     * @param input The string to hash
     * @return Hex-encoded hash
     */
    std::string GenerateSecureHash(const std::string& input);

    /**
     * @brief Generate HMAC-SHA256 for data integrity
     * @param data The data to authenticate
     * @param key The secret key
     * @return Hex-encoded HMAC
     */
    std::string GenerateHMAC(const std::string& data, const std::string& key);

    /**
     * @brief Verify HMAC-SHA256
     * @param data The original data
     * @param key The secret key
     * @param providedMAC The HMAC to verify
     * @return true if HMAC is valid
     */
    bool VerifyHMAC(const std::string& data, const std::string& key, const std::string& providedMAC);

    /**
     * @brief Generate a cryptographically secure random key
     * @param keySize Size of the key in bytes
     * @return Hex-encoded random key
     */
    std::string GenerateRandomKey(size_t keySize = 32);

    /**
     * @brief Set the encryption mode
     * @param mode The encryption mode to use
     */
    void SetEncryptionMode(EncryptionMode mode);

    /**
     * @brief Get the current public key (for RSA mode)
     * @return Base64-encoded public key
     */
    std::string GetPublicKey();

    /**
     * @brief Set a custom public key for encryption (RSA mode)
     * @param publicKey Base64-encoded public key
     * @return true if key was set successfully
     */
    bool SetPublicKey(const std::string& publicKey);

    /**
     * @brief Export the private key (for backup purposes)
     * @param password Password to encrypt the private key
     * @return Encrypted private key
     */
    std::string ExportPrivateKey(const std::string& password);

    /**
     * @brief Import a private key
     * @param encryptedPrivateKey Encrypted private key
     * @param password Password to decrypt the private key
     * @return true if key was imported successfully
     */
    bool ImportPrivateKey(const std::string& encryptedPrivateKey, const std::string& password);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

    // AES encryption methods
    std::string EncryptStringAES_CBC(const std::string& plaintext, const std::string& password);
    std::string DecryptStringAES_CBC(const std::string& ciphertext, const std::string& password);
    std::string EncryptStringAES_GCM(const std::string& plaintext, const std::string& password);
    std::string DecryptStringAES_GCM(const std::string& ciphertext, const std::string& password);

    // RSA encryption methods
    std::string EncryptStringRSA(const std::string& plaintext);
    std::string DecryptStringRSA(const std::string& ciphertext);

    // Utility methods
    void DeriveKeyFromPassword(const std::string& password, 
                              const CryptoPP::SecByteBlock& salt, 
                              CryptoPP::SecByteBlock& derivedKey);
    void GenerateRSAKeyPair();
};

} // namespace Aether::Security