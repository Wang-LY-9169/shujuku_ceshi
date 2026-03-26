#include "sql_parser.h"
#include <algorithm>
#include <sstream>

namespace MemoryDB {

SQLParser::SQLParser() {
}

SQLParser::~SQLParser() {
}

std::optional<SQLCommand> SQLParser::parse(const std::string& sql) {
    SQLCommand cmd;
    std::string upperSql = toUpper(sql);
    std::string trimmedSql = trim(sql);

    if (startsWith(upperSql, "CREATE TABLE")) {
        return parseCreateTable(trimmedSql);
    } else if (startsWith(upperSql, "DROP TABLE")) {
        return parseDropTable(trimmedSql);
    } else if (startsWith(upperSql, "INSERT INTO")) {
        return parseInsert(trimmedSql);
    } else if (startsWith(upperSql, "SELECT")) {
        return parseSelect(trimmedSql);
    } else if (startsWith(upperSql, "UPDATE")) {
        return parseUpdate(trimmedSql);
    } else if (startsWith(upperSql, "DELETE")) {
        return parseDelete(trimmedSql);
    }

    return std::nullopt;
}

bool SQLParser::validate(const std::string& sql) {
    return parse(sql).has_value();
}

std::optional<SQLCommand> SQLParser::parseCreateTable(const std::string& sql) {
    SQLCommand cmd;
    cmd.type = SQLCommandType::CREATE_TABLE;

    // 格式: CREATE TABLE table_name (col1 type1, col2 type2, ...)
    size_t tableStart = 12;  // "CREATE TABLE "
    size_t tableEnd = sql.find('(');
    if (tableEnd == std::string::npos) {
        return std::nullopt;
    }

    cmd.tableName = trim(sql.substr(tableStart, tableEnd - tableStart));

    // 解析列定义
    std::string colsStr = sql.substr(tableEnd + 1);
    size_t lastParen = colsStr.rfind(')');
    if (lastParen != std::string::npos) {
        colsStr = colsStr.substr(0, lastParen);
    }

    std::vector<std::string> colDefs = split(colsStr, ',');
    for (auto& colDef : colDefs) {
        colDef = trim(colDef);
        std::vector<std::string> parts = split(colDef, ' ');
        if (parts.size() >= 1) {
            cmd.columns.push_back(parts[0]);  // 只存列名
        }
    }

    return cmd;
}

std::optional<SQLCommand> SQLParser::parseDropTable(const std::string& sql) {
    SQLCommand cmd;
    cmd.type = SQLCommandType::DROP_TABLE;

    // 格式: DROP TABLE table_name
    std::vector<std::string> parts = split(trim(sql), ' ');
    if (parts.size() >= 3) {
        cmd.tableName = parts[2];
        return cmd;
    }

    return std::nullopt;
}

std::optional<SQLCommand> SQLParser::parseInsert(const std::string& sql) {
    SQLCommand cmd;
    cmd.type = SQLCommandType::INSERT;

    // 格式: INSERT INTO table_name VALUES (val1, val2, ...)
    size_t tableNameEnd = sql.find(" VALUES");
    if (tableNameEnd == std::string::npos) {
        return std::nullopt;
    }

    // 提取表名
    size_t tableNameStart = 11;  // "INSERT INTO "
    cmd.tableName = trim(sql.substr(tableNameStart, tableNameEnd - tableNameStart));

    // 提取值
    size_t valuesStart = tableNameEnd + 7;  // " VALUES "
    size_t firstParen = sql.find('(', valuesStart);
    size_t lastParen = sql.rfind(')');

    if (firstParen == std::string::npos || lastParen == std::string::npos) {
        return std::nullopt;
    }

    std::string valuesStr = sql.substr(firstParen + 1, lastParen - firstParen - 1);
    cmd.values = split(valuesStr, ',');

    // 去除值的引号
    for (auto& val : cmd.values) {
        val = trim(val);
        if (val.size() >= 2 && val.front() == '\'' && val.back() == '\'') {
            val = val.substr(1, val.size() - 2);
        }
    }

    return cmd;
}

std::optional<SQLCommand> SQLParser::parseSelect(const std::string& sql) {
    SQLCommand cmd;
    cmd.type = SQLCommandType::SELECT;

    // 格式: SELECT columns FROM table_name [WHERE condition]
    size_t fromPos = toUpper(sql).find(" FROM ");
    if (fromPos == std::string::npos) {
        return std::nullopt;
    }

    // 提取列
    std::string colsStr = trim(sql.substr(6, fromPos - 6));  // "SELECT "
    if (colsStr == "*") {
        cmd.columns.push_back("*");
    } else {
        cmd.columns = split(colsStr, ',');
        for (auto& col : cmd.columns) {
            col = trim(col);
        }
    }

    // 提取表名和WHERE
    std::string rest = trim(sql.substr(fromPos + 6));
    size_t wherePos = toUpper(rest).find(" WHERE ");
    if (wherePos != std::string::npos) {
        cmd.tableName = trim(rest.substr(0, wherePos));
        cmd.condition = trim(rest.substr(wherePos + 6));
    } else {
        cmd.tableName = rest;
    }

    return cmd;
}

std::optional<SQLCommand> SQLParser::parseUpdate(const std::string& sql) {
    SQLCommand cmd;
    cmd.type = SQLCommandType::UPDATE;

    // 格式: UPDATE table_name SET col1=val1, col2=val2 [WHERE condition]
    size_t setPos = toUpper(sql).find(" SET ");
    if (setPos == std::string::npos) {
        return std::nullopt;
    }

    cmd.tableName = trim(sql.substr(6, setPos - 6));  // "UPDATE "

    std::string rest = trim(sql.substr(setPos + 5));
    size_t wherePos = toUpper(rest).find(" WHERE ");

    std::string setStr;
    if (wherePos != std::string::npos) {
        setStr = trim(rest.substr(0, wherePos));
        cmd.condition = trim(rest.substr(wherePos + 6));
    } else {
        setStr = rest;
    }

    // 解析 SET
    std::vector<std::string> sets = split(setStr, ',');
    for (auto& setItem : sets) {
        setItem = trim(setItem);
        size_t eqPos = setItem.find('=');
        if (eqPos != std::string::npos) {
            cmd.columns.push_back(trim(setItem.substr(0, eqPos)));
            std::string val = trim(setItem.substr(eqPos + 1));
            if (val.size() >= 2 && val.front() == '\'' && val.back() == '\'') {
                val = val.substr(1, val.size() - 2);
            }
            cmd.values.push_back(val);
        }
    }

    return cmd;
}

std::optional<SQLCommand> SQLParser::parseDelete(const std::string& sql) {
    SQLCommand cmd;
    cmd.type = SQLCommandType::DELETE;

    // 格式: DELETE FROM table_name [WHERE condition]
    size_t fromPos = toUpper(sql).find(" FROM ");
    if (fromPos == std::string::npos) {
        return std::nullopt;
    }

    std::string rest = trim(sql.substr(fromPos + 6));
    size_t wherePos = toUpper(rest).find(" WHERE ");

    if (wherePos != std::string::npos) {
        cmd.tableName = trim(rest.substr(0, wherePos));
        cmd.condition = trim(rest.substr(wherePos + 6));
    } else {
        cmd.tableName = rest;
    }

    return cmd;
}

std::string SQLParser::toUpper(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> SQLParser::split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

std::string SQLParser::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool SQLParser::startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() &&
           toUpper(str.substr(0, prefix.size())) == toUpper(prefix);
}

} // namespace MemoryDB
