#ifndef MEMORYDB_ENCRYPTION_H
#define MEMORYDB_ENCRYPTION_H

#include <string>
#include <vector>

namespace MemoryDB {

// 加密引擎
class CryptoEngine {
public:
    // 构造函数：使用密钥初始化
    CryptoEngine(const std::string& key);
    ~CryptoEngine();

    // 加密字符串
    std::string encrypt(const std::string& plaintext);

    // 解密字符串
    std::string decrypt(const std::string& ciphertext);

    // 检查加密是否启用
    bool isEnabled() const { return !key_.empty(); }

private:
    std::string key_;  // 加密密钥

    // XOR 加密实现
    std::string xorEncryptDecrypt(const std::string& data);
};

} // namespace MemoryDB

#endif // MEMORYDB_ENCRYPTION_H
