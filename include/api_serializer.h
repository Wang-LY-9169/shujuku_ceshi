#ifndef MEMORYDB_API_SERIALIZER_H
#define MEMORYDB_API_SERIALIZER_H

#include "database.h"
#include <string>
#include <vector>

namespace MemoryDB {

// API 响应序列化器
class ApiSerializer {
public:
    // 将 QueryResult 序列化为 JSON 字符串
    static std::string serialize(const QueryResult& result);

    // 生成统一错误响应 JSON
    static std::string error(const std::string& message, int errorCode = 500);

private:
    // JSON 字符串转义
    static std::string escapeString(const std::string& str);

    // 序列化 JSON 数组
    static std::string serializeArray(const std::vector<std::string>& arr);

    // 序列化 JSON 二维数组
    static std::string serializeRows(const std::vector<std::vector<std::string>>& rows);
};

} // namespace MemoryDB

#endif // MEMORYDB_API_SERIALIZER_H
