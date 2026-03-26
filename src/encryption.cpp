#include "encryption.h"
#include <algorithm>

namespace MemoryDB {

CryptoEngine::CryptoEngine(const std::string& key)
    : key_(key) {
}

CryptoEngine::~CryptoEngine() {
}

std::string CryptoEngine::encrypt(const std::string& plaintext) {
    if (key_.empty()) {
        return plaintext;  // 无密钥，不加密
    }
    return xorEncryptDecrypt(plaintext);
}

std::string CryptoEngine::decrypt(const std::string& ciphertext) {
    if (key_.empty()) {
        return ciphertext;  // 无密钥，不解密
    }
    return xorEncryptDecrypt(ciphertext);  // XOR 对称，加密=解密
}

std::string CryptoEngine::xorEncryptDecrypt(const std::string& data) {
    std::string result = data;
    size_t keyLen = key_.length();

    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = data[i] ^ key_[i % keyLen];
    }

    return result;
}

} // namespace MemoryDB
