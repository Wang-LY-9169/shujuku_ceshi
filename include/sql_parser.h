#ifndef MEMORYDB_SQL_PARSER_H
#define MEMORYDB_SQL_PARSER_H

#include <string>
#include <vector>
#include <optional>

namespace MemoryDB {

// SQL 命令类型
enum class SQLCommandType {
    CREATE_TABLE,
    DROP_TABLE,
    INSERT,
    SELECT,
    UPDATE,
    DELETE,
    UNKNOWN
};

// SQL 命令结构
struct SQLCommand {
    SQLCommandType type;           // 命令类型
    std::string tableName;         // 表名
    std::vector<std::string> columns;  // 列名
    std::vector<std::string> values;    // 值
    std::string condition;             // 条件（WHERE子句）

    SQLCommand() : type(SQLCommandType::UNKNOWN) {}
};

// SQL 解析器
class SQLParser {
public:
    SQLParser();
    ~SQLParser();

    // 解析 SQL 语句
    std::optional<SQLCommand> parse(const std::string& sql);

    // 检查 SQL 语法是否有效
    bool validate(const std::string& sql);

private:
    // 解析 CREATE TABLE
    std::optional<SQLCommand> parseCreateTable(const std::string& sql);

    // 解析 DROP TABLE
    std::optional<SQLCommand> parseDropTable(const std::string& sql);

    // 解析 INSERT
    std::optional<SQLCommand> parseInsert(const std::string& sql);

    // 解析 SELECT
    std::optional<SQLCommand> parseSelect(const std::string& sql);

    // 解析 UPDATE
    std::optional<SQLCommand> parseUpdate(const std::string& sql);

    // 解析 DELETE
    std::optional<SQLCommand> parseDelete(const std::string& sql);

    // 辅助函数
    std::string toUpper(const std::string& s);
    std::vector<std::string> split(const std::string& s, char delim);
    std::string trim(const std::string& s);
    bool startsWith(const std::string& str, const std::string& prefix);
};

} // namespace MemoryDB

#endif // MEMORYDB_SQL_PARSER_H
