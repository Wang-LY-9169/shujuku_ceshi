#ifndef MEMORYDB_DATABASE_H
#define MEMORYDB_DATABASE_H

#include "storage.h"
#include "sql_parser.h"
#include "encryption.h"
#include <string>
#include <memory>
#include <vector>

namespace MemoryDB {

// 查询结果
struct QueryResult {
    bool success = true;                           // 是否成功
    std::string message;                            // 结果消息
    int affectedRows = 0;                           // 影响行数
    std::vector<std::string> columns;              // 列名（SELECT时使用）
    std::vector<std::vector<std::string>> rows;    // 数据行（SELECT时使用）
};

// Database 核心调度器
class Database {
public:
    Database();
    ~Database();

    // 执行 SQL 语句
    QueryResult execute(const std::string& sql);

    // 列出所有表名
    std::vector<std::string> listTables() const;

    // 设置加密密钥
    void setEncryptionKey(const std::string& key);

private:
    StorageEngine storage_;                          // 存储引擎
    SQLParser parser_;                              // SQL 解析器
    std::shared_ptr<CryptoEngine> crypto_;           // 加密引擎

    // 执行 CREATE TABLE
    QueryResult executeCreateTable(const SQLCommand& cmd);

    // 执行 DROP TABLE
    QueryResult executeDropTable(const SQLCommand& cmd);

    // 执行 INSERT
    QueryResult executeInsert(const SQLCommand& cmd);

    // 执行 SELECT
    QueryResult executeSelect(const SQLCommand& cmd);

    // 执行 UPDATE
    QueryResult executeUpdate(const SQLCommand& cmd);

    // 执行 DELETE
    QueryResult executeDelete(const SQLCommand& cmd);

    // 字符串转列定义
    std::vector<ColumnDef> parseColumns(const SQLCommand& cmd);
};

} // namespace MemoryDB

#endif // MEMORYDB_DATABASE_H
