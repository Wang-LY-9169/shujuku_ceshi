#ifndef MEMORYDB_STORAGE_H
#define MEMORYDB_STORAGE_H

#include "table.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace MemoryDB {

// 存储引擎：管理所有表
class StorageEngine {
public:
    StorageEngine();
    ~StorageEngine();

    // 创建表
    bool createTable(const std::string& name, const std::vector<ColumnDef>& schema);

    // 删除表
    bool dropTable(const std::string& name);

    // 获取表
    Table* getTable(const std::string& name);

    // 列出所有表名
    std::vector<std::string> listTables() const;

    // 检查表是否存在
    bool tableExists(const std::string& name) const;

private:
    // 表名到表的映射
    std::map<std::string, std::unique_ptr<Table>> tables_;
};

} // namespace MemoryDB

#endif // MEMORYDB_STORAGE_H
