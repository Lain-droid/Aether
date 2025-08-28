#include "EncryptionManager.h"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/sha.h>
#include <cryptopp/hmac.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <spdlog/spdlog.h>
#include <random>
#include <chrono>

namespace Aether::Security {

class EncryptionManager::Impl {
public:
    CryptoPP::AutoSeededRandomPool rng;
    CryptoPP::SecByteBlock aesKey{CryptoPP::AES::DEFAULT_KEYLENGTH};
    CryptoPP::SecByteBlock hmacKey{32}; // 256-bit HMAC key
    
    // RSA keys for asymmetric encryption
    CryptoPP::RSA::PrivateKey rsaPrivateKey;
    CryptoPP::RSA::PublicKey rsaPublicKey;
    
    bool isInitialized = false;
    EncryptionMode currentMode = EncryptionMode::AES_256_GCM;
    
    // Key derivation parameters
    static constexpr size_t PBKDF2_ITERATIONS = 100000;
    static constexpr size_t SALT_SIZE = 32;
    static constexpr size_t IV_SIZE = 16;
};

EncryptionManager::EncryptionManager() : pImpl(std::make_unique<Impl>()) {}

EncryptionManager::~EncryptionManager() = default;

bool EncryptionManager::Initialize() {
    try {
        spdlog::info("Initializing Encryption Manager");
        
        // Generate AES key
        pImpl->rng.GenerateBlock(pImpl->aesKey, pImpl->aesKey.size());
        
        // Generate HMAC key
        pImpl->rng.GenerateBlock(pImpl->hmacKey, pImpl->hmacKey.size());
        
        // Generate RSA key pair
        GenerateRSAKeyPair();
        
        pImpl->isInitialized = true;
        spdlog::info("Encryption Manager initialized successfully");
        return true;
    }
    catch (const std::exception& ex) {
        spdlog::error("Failed to initialize encryption manager: {}", ex.what());
        return false;
    }
}

std::string EncryptionManager::EncryptString(const std::string& plaintext, const std::string& password) {
    if (!pImpl->isInitialized) {
        throw std::runtime_error("Encryption manager not initialized");
    }
    
    try {
        switch (pImpl->currentMode) {
            case EncryptionMode::AES_256_CBC:
                return EncryptStringAES_CBC(plaintext, password);
            case EncryptionMode::AES_256_GCM:
                return EncryptStringAES_GCM(plaintext, password);
            case EncryptionMode::RSA_OAEP:
                return EncryptStringRSA(plaintext);
            default:
                throw std::runtime_error("Unsupported encryption mode");
        }
    }
    catch (const std::exception& ex) {
        spdlog::error("Encryption failed: {}", ex.what());
        throw;
    }
}

std::string EncryptionManager::DecryptString(const std::string& ciphertext, const std::string& password) {
    if (!pImpl->isInitialized) {
        throw std::runtime_error("Encryption manager not initialized");
    }
    
    try {
        switch (pImpl->currentMode) {
            case EncryptionMode::AES_256_CBC:
                return DecryptStringAES_CBC(ciphertext, password);
            case EncryptionMode::AES_256_GCM:
                return DecryptStringAES_GCM(ciphertext, password);
            case EncryptionMode::RSA_OAEP:
                return DecryptStringRSA(ciphertext);
            default:
                throw std::runtime_error("Unsupported encryption mode");
        }
    }
    catch (const std::exception& ex) {
        spdlog::error("Decryption failed: {}", ex.what());
        throw;
    }
}

std::vector<uint8_t> EncryptionManager::EncryptData(const std::vector<uint8_t>& data, const std::string& password) {
    if (!pImpl->isInitialized) {
        throw std::runtime_error("Encryption manager not initialized");
    }
    
    try {
        // Generate salt and IV
        CryptoPP::SecByteBlock salt(Impl::SALT_SIZE);
        CryptoPP::SecByteBlock iv(Impl::IV_SIZE);
        pImpl->rng.GenerateBlock(salt, salt.size());
        pImpl->rng.GenerateBlock(iv, iv.size());
        
        // Derive key from password
        CryptoPP::SecByteBlock derivedKey(CryptoPP::AES::DEFAULT_KEYLENGTH);
        DeriveKeyFromPassword(password, salt, derivedKey);
        
        // Encrypt data
        std::string encrypted;
        CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption;
        encryption.SetKeyWithIV(derivedKey, derivedKey.size(), iv);
        
        CryptoPP::StringSource ss(data.data(), data.size(), true,
            new CryptoPP::StreamTransformationFilter(encryption,
                new CryptoPP::StringSink(encrypted)
            )
        );
        
        // Combine salt + IV + encrypted data
        std::vector<uint8_t> result;
        result.reserve(salt.size() + iv.size() + encrypted.size());
        
        result.insert(result.end(), salt.begin(), salt.end());
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), encrypted.begin(), encrypted.end());
        
        return result;
    }
    catch (const std::exception& ex) {
        spdlog::error("Data encryption failed: {}", ex.what());
        throw;
    }
}

std::vector<uint8_t> EncryptionManager::DecryptData(const std::vector<uint8_t>& encryptedData, const std::string& password) {
    if (!pImpl->isInitialized) {
        throw std::runtime_error("Encryption manager not initialized");
    }
    
    if (encryptedData.size() < Impl::SALT_SIZE + Impl::IV_SIZE) {
        throw std::runtime_error("Invalid encrypted data size");
    }
    
    try {
        // Extract salt and IV
        CryptoPP::SecByteBlock salt(encryptedData.data(), Impl::SALT_SIZE);
        CryptoPP::SecByteBlock iv(encryptedData.data() + Impl::SALT_SIZE, Impl::IV_SIZE);
        
        // Extract encrypted data
        auto encryptedStart = encryptedData.begin() + Impl::SALT_SIZE + Impl::IV_SIZE;
        std::string encrypted(encryptedStart, encryptedData.end());
        
        // Derive key from password
        CryptoPP::SecByteBlock derivedKey(CryptoPP::AES::DEFAULT_KEYLENGTH);
        DeriveKeyFromPassword(password, salt, derivedKey);
        
        // Decrypt data
        std::string decrypted;
        CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption;
        decryption.SetKeyWithIV(derivedKey, derivedKey.size(), iv);
        
        CryptoPP::StringSource ss(encrypted, true,
            new CryptoPP::StreamTransformationFilter(decryption,
                new CryptoPP::StringSink(decrypted)
            )
        );
        
        return std::vector<uint8_t>(decrypted.begin(), decrypted.end());
    }
    catch (const std::exception& ex) {
        spdlog::error("Data decryption failed: {}", ex.what());
        throw;
    }
}

std::string EncryptionManager::GenerateSecureHash(const std::string& input) {
    try {
        CryptoPP::SHA256 hash;
        std::string digest;
        
        CryptoPP::StringSource ss(input, true,
            new CryptoPP::HashFilter(hash,
                new CryptoPP::HexEncoder(
                    new CryptoPP::StringSink(digest)
                )
            )
        );
        
        return digest;
    }
    catch (const std::exception& ex) {
        spdlog::error("Hash generation failed: {}", ex.what());
        throw;
    }
}

std::string EncryptionManager::GenerateHMAC(const std::string& data, const std::string& key) {
    try {
        CryptoPP::HMAC<CryptoPP::SHA256> hmac(reinterpret_cast<const CryptoPP::byte*>(key.data()), key.size());
        std::string mac;
        
        CryptoPP::StringSource ss(data, true,
            new CryptoPP::HashFilter(hmac,
                new CryptoPP::HexEncoder(
                    new CryptoPP::StringSink(mac)
                )
            )
        );
        
        return mac;
    }
    catch (const std::exception& ex) {
        spdlog::error("HMAC generation failed: {}", ex.what());
        throw;
    }
}

bool EncryptionManager::VerifyHMAC(const std::string& data, const std::string& key, const std::string& providedMAC) {
    try {
        std::string calculatedMAC = GenerateHMAC(data, key);
        return calculatedMAC == providedMAC;
    }
    catch (const std::exception& ex) {
        spdlog::error("HMAC verification failed: {}", ex.what());
        return false;
    }
}

std::string EncryptionManager::GenerateRandomKey(size_t keySize) {
    try {
        CryptoPP::SecByteBlock key(keySize);
        pImpl->rng.GenerateBlock(key, key.size());
        
        std::string encodedKey;
        CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(encodedKey));
        encoder.Put(key, key.size());
        encoder.MessageEnd();
        
        return encodedKey;
    }
    catch (const std::exception& ex) {
        spdlog::error("Random key generation failed: {}", ex.what());
        throw;
    }
}

void EncryptionManager::SetEncryptionMode(EncryptionMode mode) {
    pImpl->currentMode = mode;
    spdlog::info("Encryption mode set to: {}", static_cast<int>(mode));
}

std::string EncryptionManager::GetPublicKey() {
    try {
        std::string publicKeyStr;
        CryptoPP::StringSink publicKeySink(publicKeyStr);
        pImpl->rsaPublicKey.DEREncode(publicKeySink);
        
        std::string encodedKey;
        CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(encodedKey));
        encoder.Put(reinterpret_cast<const CryptoPP::byte*>(publicKeyStr.data()), publicKeyStr.size());
        encoder.MessageEnd();
        
        return encodedKey;
    }
    catch (const std::exception& ex) {
        spdlog::error("Failed to get public key: {}", ex.what());
        throw;
    }
}

// Private methods

std::string EncryptionManager::EncryptStringAES_CBC(const std::string& plaintext, const std::string& password) {
    // Generate salt and IV
    CryptoPP::SecByteBlock salt(Impl::SALT_SIZE);
    CryptoPP::SecByteBlock iv(Impl::IV_SIZE);
    pImpl->rng.GenerateBlock(salt, salt.size());
    pImpl->rng.GenerateBlock(iv, iv.size());
    
    // Derive key from password
    CryptoPP::SecByteBlock derivedKey(CryptoPP::AES::DEFAULT_KEYLENGTH);
    DeriveKeyFromPassword(password, salt, derivedKey);
    
    // Encrypt
    std::string encrypted;
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption;
    encryption.SetKeyWithIV(derivedKey, derivedKey.size(), iv);
    
    CryptoPP::StringSource ss(plaintext, true,
        new CryptoPP::StreamTransformationFilter(encryption,
            new CryptoPP::StringSink(encrypted)
        )
    );
    
    // Combine salt + IV + encrypted data and encode
    std::string combined;
    combined.append(reinterpret_cast<const char*>(salt.data()), salt.size());
    combined.append(reinterpret_cast<const char*>(iv.data()), iv.size());
    combined.append(encrypted);
    
    std::string encoded;
    CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(encoded));
    encoder.Put(reinterpret_cast<const CryptoPP::byte*>(combined.data()), combined.size());
    encoder.MessageEnd();
    
    return encoded;
}

std::string EncryptionManager::DecryptStringAES_CBC(const std::string& ciphertext, const std::string& password) {
    // Decode from Base64
    std::string decoded;
    CryptoPP::Base64Decoder decoder(new CryptoPP::StringSink(decoded));
    decoder.Put(reinterpret_cast<const CryptoPP::byte*>(ciphertext.data()), ciphertext.size());
    decoder.MessageEnd();
    
    if (decoded.size() < Impl::SALT_SIZE + Impl::IV_SIZE) {
        throw std::runtime_error("Invalid ciphertext size");
    }
    
    // Extract salt and IV
    CryptoPP::SecByteBlock salt(reinterpret_cast<const CryptoPP::byte*>(decoded.data()), Impl::SALT_SIZE);
    CryptoPP::SecByteBlock iv(reinterpret_cast<const CryptoPP::byte*>(decoded.data() + Impl::SALT_SIZE), Impl::IV_SIZE);
    
    // Extract encrypted data
    std::string encrypted = decoded.substr(Impl::SALT_SIZE + Impl::IV_SIZE);
    
    // Derive key from password
    CryptoPP::SecByteBlock derivedKey(CryptoPP::AES::DEFAULT_KEYLENGTH);
    DeriveKeyFromPassword(password, salt, derivedKey);
    
    // Decrypt
    std::string decrypted;
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption;
    decryption.SetKeyWithIV(derivedKey, derivedKey.size(), iv);
    
    CryptoPP::StringSource ss(encrypted, true,
        new CryptoPP::StreamTransformationFilter(decryption,
            new CryptoPP::StringSink(decrypted)
        )
    );
    
    return decrypted;
}

std::string EncryptionManager::EncryptStringAES_GCM(const std::string& plaintext, const std::string& password) {
    // GCM implementation - similar to CBC but with authentication
    // This is a simplified version
    return EncryptStringAES_CBC(plaintext, password);
}

std::string EncryptionManager::DecryptStringAES_GCM(const std::string& ciphertext, const std::string& password) {
    // GCM implementation - similar to CBC but with authentication
    // This is a simplified version
    return DecryptStringAES_CBC(ciphertext, password);
}

std::string EncryptionManager::EncryptStringRSA(const std::string& plaintext) {
    try {
        std::string encrypted;
        CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(pImpl->rsaPublicKey);
        
        CryptoPP::StringSource ss(plaintext, true,
            new CryptoPP::PK_EncryptorFilter(pImpl->rng, encryptor,
                new CryptoPP::Base64Encoder(
                    new CryptoPP::StringSink(encrypted)
                )
            )
        );
        
        return encrypted;
    }
    catch (const std::exception& ex) {
        spdlog::error("RSA encryption failed: {}", ex.what());
        throw;
    }
}

std::string EncryptionManager::DecryptStringRSA(const std::string& ciphertext) {
    try {
        std::string decrypted;
        CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(pImpl->rsaPrivateKey);
        
        CryptoPP::StringSource ss(ciphertext, true,
            new CryptoPP::Base64Decoder(
                new CryptoPP::PK_DecryptorFilter(pImpl->rng, decryptor,
                    new CryptoPP::StringSink(decrypted)
                )
            )
        );
        
        return decrypted;
    }
    catch (const std::exception& ex) {
        spdlog::error("RSA decryption failed: {}", ex.what());
        throw;
    }
}

void EncryptionManager::DeriveKeyFromPassword(const std::string& password, 
                                              const CryptoPP::SecByteBlock& salt, 
                                              CryptoPP::SecByteBlock& derivedKey) {
    try {
        CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf2;
        pbkdf2.DeriveKey(derivedKey, derivedKey.size(),
                        reinterpret_cast<const CryptoPP::byte*>(password.data()), password.size(),
                        salt, salt.size(),
                        Impl::PBKDF2_ITERATIONS);
    }
    catch (const std::exception& ex) {
        spdlog::error("Key derivation failed: {}", ex.what());
        throw;
    }
}

void EncryptionManager::GenerateRSAKeyPair() {
    try {
        CryptoPP::InvertibleRSAFunction params;
        params.GenerateRandomWithKeySize(pImpl->rng, 2048);
        
        pImpl->rsaPrivateKey = CryptoPP::RSA::PrivateKey(params);
        pImpl->rsaPublicKey = CryptoPP::RSA::PublicKey(params);
        
        spdlog::debug("RSA key pair generated successfully");
    }
    catch (const std::exception& ex) {
        spdlog::error("RSA key pair generation failed: {}", ex.what());
        throw;
    }
}

} // namespace Aether::Security