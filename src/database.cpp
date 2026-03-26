#include "database.h"
#include <iostream>
#include "storage.h"

namespace MemoryDB {

Database::Database() {
}

Database::~Database() {
}

QueryResult Database::execute(const std::string& sql) {
    auto cmd = parser_.parse(sql);
    if (!cmd) {
        QueryResult result;
        result.success = false;
        result.message = "SQL语法错误: " + sql;
        return result;
    }

    switch (cmd->type) {
        case SQLCommandType::CREATE_TABLE:
            return executeCreateTable(*cmd);
        case SQLCommandType::DROP_TABLE:
            return executeDropTable(*cmd);
        case SQLCommandType::INSERT:
            return executeInsert(*cmd);
        case SQLCommandType::SELECT:
            return executeSelect(*cmd);
        case SQLCommandType::UPDATE:
            return executeUpdate(*cmd);
        case SQLCommandType::DELETE:
            return executeDelete(*cmd);
        default:
            QueryResult result;
            result.success = false;
            result.message = "不支持的SQL命令";
            return result;
    }
}

std::vector<std::string> Database::listTables() const {
    return storage_.listTables();
}

QueryResult Database::executeCreateTable(const SQLCommand& cmd) {
    QueryResult result;

    auto columns = parseColumns(cmd);
    if (!storage_.createTable(cmd.tableName, columns)) {
        result.success = false;
        result.message = "创建表失败: 表 '" + cmd.tableName + "' 已存在";
        return result;
    }

    // 为表设置加密引擎
    Table* table = storage_.getTable(cmd.tableName);
    if (table && crypto_) {
        table->setCryptoEngine(crypto_);
    }

    result.message = "表 '" + cmd.tableName + "' 创建成功";
    if (crypto_ && crypto_->isEnabled()) {
        result.message += " (已启用加密)";
    }
    return result;
}

QueryResult Database::executeDropTable(const SQLCommand& cmd) {
    QueryResult result;

    if (!storage_.dropTable(cmd.tableName)) {
        result.success = false;
        result.message = "删除表失败: 表 '" + cmd.tableName + "' 不存在";
        return result;
    }

    result.message = "表 '" + cmd.tableName + "' 删除成功";
    return result;
}

QueryResult Database::executeInsert(const SQLCommand& cmd) {
    QueryResult result;

    Table* table = storage_.getTable(cmd.tableName);
    if (!table) {
        result.success = false;
        result.message = "插入失败: 表 '" + cmd.tableName + "' 不存在";
        return result;
    }

    // 创建行并插入
    Row row;
    for (const auto& value : cmd.values) {
        row.push_back(value);
    }

    if (table->insertRow(row)) {
        result.affectedRows = 1;
        result.message = "插入成功";
    } else {
        result.success = false;
        result.message = "插入失败: 数据不匹配表结构";
    }

    return result;
}

QueryResult Database::executeSelect(const SQLCommand& cmd) {
    QueryResult result;

    Table* table = storage_.getTable(cmd.tableName);
    if (!table) {
        result.success = false;
        result.message = "查询失败: 表 '" + cmd.tableName + "' 不存在";
        return result;
    }

    // 获取表结构（列名）
    auto schema = table->getSchema();
    for (const auto& col : schema) {
        result.columns.push_back(col.name);
    }

    // 获取所有行（WHERE条件暂时忽略）
    auto tableRows = table->selectRows();
    result.affectedRows = static_cast<int>(tableRows.size());
    result.rows = tableRows;
    result.message = "查询成功，共 " + std::to_string(tableRows.size()) + " 行";

    return result;
}

QueryResult Database::executeUpdate(const SQLCommand& cmd) {
    QueryResult result;

    Table* table = storage_.getTable(cmd.tableName);
    if (!table) {
        result.success = false;
        result.message = "更新失败: 表 '" + cmd.tableName + "' 不存在";
        return result;
    }

    // 简化处理：只处理第一个列
    if (cmd.columns.empty() || cmd.values.empty()) {
        result.success = false;
        result.message = "更新失败: 缺少列或值";
        return result;
    }

    // 查找列索引（简化：假设列名与顺序一致）
    int colIndex = 0;  // 暂时固定为第一列

    if (table->updateRows(colIndex, cmd.values[0])) {
        result.affectedRows = table->getRowCount();
        result.message = "更新成功，影响 " + std::to_string(result.affectedRows) + " 行";
    } else {
        result.success = false;
        result.message = "更新失败: 列索引无效";
    }

    return result;
}

QueryResult Database::executeDelete(const SQLCommand& cmd) {
    QueryResult result;

    Table* table = storage_.getTable(cmd.tableName);
    if (!table) {
        result.success = false;
        result.message = "删除失败: 表 '" + cmd.tableName + "' 不存在";
        return result;
    }

    size_t oldCount = table->getRowCount();

    if (table->deleteRows()) {
        result.affectedRows = oldCount;
        result.message = "删除成功，共 " + std::to_string(oldCount) + " 行";
    } else {
        result.success = false;
        result.message = "删除失败";
    }

    return result;
}

std::vector<ColumnDef> Database::parseColumns(const SQLCommand& cmd) {
    std::vector<ColumnDef> columns;

    // 简化处理：从CREATE TABLE解析中获取列信息
    // 实际应该在Parser中完成，这里简单处理
    for (size_t i = 0; i < cmd.columns.size(); ++i) {
        ColumnDef col;
        col.name = cmd.columns[i];
        col.type = "TEXT";  // 默认类型
        columns.push_back(col);
    }

    return columns;
}

void Database::setEncryptionKey(const std::string& key) {
    if (!key.empty()) {
        crypto_ = std::make_shared<CryptoEngine>(key);
    } else {
        crypto_.reset();
    }
}

} // namespace MemoryDB
