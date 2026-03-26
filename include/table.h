#ifndef MEMORYDB_TABLE_H
#define MEMORYDB_TABLE_H

#include <string>
#include <vector>
#include <memory>

namespace MemoryDB {

// 前向声明
class CryptoEngine;

// 数据行类型：使用 vector<string>
using Row = std::vector<std::string>;

// 表结构定义
struct ColumnDef {
    std::string name;      // 列名
    std::string type;      // 数据类型 (INT, TEXT, REAL, BLOB)
};

class Table {
public:
    // 构造函数：创建表
    Table(const std::string& name);

    // 设置表结构
    void setSchema(const std::vector<ColumnDef>& columns);

    // 获取表名
    std::string getName() const;

    // 获取列定义
    std::vector<ColumnDef> getSchema() const;

    // 获取行数
    size_t getRowCount() const;

    // 插入一行
    bool insertRow(const Row& row);

    // 查询所有行
    std::vector<Row> selectRows() const;

    // 更新行（简化版：更新指定列的所有行）
    bool updateRows(int columnIndex, const std::string& newValue);

    // 删除所有行
    bool deleteRows();

    // 设置加密引擎
    void setCryptoEngine(std::shared_ptr<CryptoEngine> engine);

    // 获取加密状态
    bool isEncrypted() const;

    // 打印表内容
    void print() const;

private:
    std::string name_;                          // 表名
    std::vector<ColumnDef> columns_;            // 列定义
    std::vector<Row> rows_;                    // 数据行
    std::shared_ptr<CryptoEngine> crypto_;      // 加密引擎
};

} // namespace MemoryDB

#endif // MEMORYDB_TABLE_H
