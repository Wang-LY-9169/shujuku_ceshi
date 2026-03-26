#include "database.h"
#include <iostream>
#include <string>
#include <algorithm>

using namespace MemoryDB;

void printBanner() {
    std::cout << "=====================================\n";
    std::cout << "       MemoryDB v1.0\n";
    std::cout << "       轻量级内存数据库\n";
    std::cout << "=====================================\n\n";
}

void printHelp() {
    std::cout << "\n=== MemoryDB 帮助 ===\n\n";
    std::cout << "SQL 命令:\n";
    std::cout << "  CREATE TABLE name (col1 type1, col2 type2, ...)\n";
    std::cout << "  DROP TABLE name\n";
    std::cout << "  INSERT INTO name VALUES (val1, val2, ...)\n";
    std::cout << "  SELECT * FROM name\n";
    std::cout << "  UPDATE name SET col=val\n";
    std::cout << "  DELETE FROM name\n\n";
    std::cout << "系统命令:\n";
    std::cout << "  help      - 显示帮助\n";
    std::cout << "  tables    - 列出所有表\n";
    std::cout << "  exit      - 退出程序\n\n";
}

// 打印 SELECT 查询结果
void printSelectResult(const QueryResult& result) {
    if (result.columns.empty()) {
        std::cout << "无数据\n";
        return;
    }

    // 计算每列宽度
    std::vector<size_t> columnWidths;
    for (const auto& col : result.columns) {
        columnWidths.push_back(col.length());
    }
    for (const auto& row : result.rows) {
        for (size_t i = 0; i < row.size() && i < columnWidths.size(); ++i) {
            columnWidths[i] = std::max(columnWidths[i], row[i].length());
        }
    }

    // 计算总宽度
    size_t totalWidth = columnWidths.size() + 1;  // 边框和分隔符
    for (size_t w : columnWidths) {
        totalWidth += w + 2;
    }

    // 打印上边框
    for (size_t i = 0; i < totalWidth; ++i) {
        std::cout << '-';
    }
    std::cout << '\n';

    // 打印列名
    std::cout << '|';
    for (size_t i = 0; i < result.columns.size(); ++i) {
        std::cout << ' ' << result.columns[i];
        for (size_t j = result.columns[i].length(); j < columnWidths[i]; ++j) {
            std::cout << ' ';
        }
        std::cout << " |";
    }
    std::cout << '\n';

    // 打印分隔线
    for (size_t i = 0; i < totalWidth; ++i) {
        std::cout << '-';
    }
    std::cout << '\n';

    // 打印数据行
    for (const auto& row : result.rows) {
        std::cout << '|';
        for (size_t i = 0; i < row.size() && i < columnWidths.size(); ++i) {
            std::cout << ' ' << row[i];
            for (size_t j = row[i].length(); j < columnWidths[i]; ++j) {
                std::cout << ' ';
            }
            std::cout << " |";
        }
        std::cout << '\n';
    }

    // 打印下边框
    for (size_t i = 0; i < totalWidth; ++i) {
        std::cout << '-';
    }
    std::cout << '\n';

    std::cout << result.rows.size() << " 行\n";
}

int main() {
    Database db;
    std::string input;

    printBanner();
    printHelp();

    // 交互式循环
    while (true) {
        std::cout << "memorydb> ";
        std::getline(std::cin, input);

        // 跳过空行
        if (input.empty()) {
            continue;
        }

        // 去除首尾空格
        size_t start = input.find_first_not_of(" \t\r\n");
        size_t end = input.find_last_not_of(" \t\r\n");
        if (start != std::string::npos) {
            input = input.substr(start, end - start + 1);
        } else {
            continue;
        }

        // 处理系统命令
        if (input == "exit" || input == "quit") {
            std::cout << "\n再见！\n";
            break;
        }

        if (input == "help" || input == "?") {
            printHelp();
            continue;
        }

        if (input == "tables" || input == "show tables") {
            auto tables = db.listTables();
            std::cout << "\n表列表 (" << tables.size() << " 个):\n";
            if (tables.empty()) {
                std::cout << "  (无)\n";
            } else {
                for (const auto& table : tables) {
                    std::cout << "  - " << table << "\n";
                }
            }
            std::cout << "\n";
            continue;
        }

        // 执行 SQL 语句
        auto result = db.execute(input);

        if (!result.success) {
            std::cout << "错误: " << result.message << "\n\n";
            continue;
        }

        // SELECT 查询需要特殊处理（展示结果表格）
        bool isSelect = (input.substr(0, 6) == "SELECT" || input.substr(0, 6) == "select");
        if (isSelect && !result.columns.empty()) {
            std::cout << "\n";
            printSelectResult(result);
            std::cout << "\n";
            continue;
        }

        // 其他操作显示消息和影响行数
        if (result.affectedRows > 0) {
            std::cout << "OK (" << result.affectedRows << " 行受影响)\n";
        } else {
            std::cout << "OK\n";
        }
        std::cout << "\n";
    }

    return 0;
}
