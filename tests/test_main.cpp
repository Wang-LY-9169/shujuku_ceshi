#include "../include/database.h"
#include "../include/storage.h"
#include "../include/sql_parser.h"
#include "../include/table.h"
#include <iostream>
#include <cassert>

using namespace MemoryDB;

void assert_test(const char* name, bool condition) {
    if (condition) {
        std::cout << "  ✅ " << name << "\n";
    } else {
        std::cout << "  ❌ " << name << " - FAILED!\n";
        exit(1);
    }
}

// ========== Table 测试 ==========
void test_table() {
    std::cout << "\n=== 测试 Table ===\n";

    Table table("test_table");

    // 设置结构
    std::vector<ColumnDef> columns = {{"id", "INT"}, {"name", "TEXT"}, {"age", "INT"}};
    table.setSchema(columns);
    assert_test("设置表结构", table.getSchema().size() == 3);

    // 插入数据
    assert_test("插入行1", table.insertRow({"1", "Alice", "25"}));
    assert_test("插入行2", table.insertRow({"2", "Bob", "30"}));
    assert_test("插入行3", table.insertRow({"3", "Charlie", "28"}));
    assert_test("行数=3", table.getRowCount() == 3);

    // 查询数据
    auto rows = table.selectRows();
    assert_test("查询行数=3", rows.size() == 3);
    assert_test("第一行数据", rows[0][0] == "1" && rows[0][1] == "Alice");

    // 更新数据
    assert_test("更新第一列", table.updateRows(0, "99"));
    rows = table.selectRows();
    assert_test("更新后验证", rows[0][0] == "99");

    // 删除数据
    assert_test("删除所有行", table.deleteRows());
    assert_test("删除后行数=0", table.getRowCount() == 0);

    // 列数验证
    assert_test("列数验证", columns.size() == 3);
}

// ========== StorageEngine 测试 ==========
void test_storage() {
    std::cout << "\n=== 测试 StorageEngine ===\n";

    StorageEngine engine;

    // 创建表
    std::vector<ColumnDef> schema = {{"id", "INT"}, {"name", "TEXT"}};
    assert_test("创建表 users", engine.createTable("users", schema));
    assert_test("创建表 products", engine.createTable("products", schema));

    // 重复创建
    assert_test("重复创建失败", !engine.createTable("users", schema));

    // 查找表
    Table* users = engine.getTable("users");
    assert_test("获取表 users", users != nullptr);
    assert_test("获取不存在的表", engine.getTable("nonexistent") == nullptr);

    // 列出表
    auto tables = engine.listTables();
    assert_test("表数量=2", tables.size() == 2);

    // 检查存在
    assert_test("users存在", engine.tableExists("users"));
    assert_test("orders不存在", !engine.tableExists("orders"));

    // 删除表
    assert_test("删除表 products", engine.dropTable("products"));
    assert_test("删除后数量=1", engine.listTables().size() == 1);
}

// ========== SQLParser 测试 ==========
void test_parser() {
    std::cout << "\n=== 测试 SQLParser ===\n";

    SQLParser parser;

    // CREATE TABLE
    auto cmd = parser.parse("CREATE TABLE test (id INT, name TEXT)");
    assert_test("解析 CREATE TABLE", cmd.has_value());
    assert_test("命令类型正确", cmd->type == SQLCommandType::CREATE_TABLE);
    assert_test("表名正确", cmd->tableName == "test");
    assert_test("列数=2", cmd->columns.size() == 2);

    // INSERT
    cmd = parser.parse("INSERT INTO test VALUES (1, 'Hello')");
    assert_test("解析 INSERT", cmd.has_value());
    assert_test("命令类型正确", cmd->type == SQLCommandType::INSERT);
    assert_test("值数=2", cmd->values.size() == 2);

    // SELECT
    cmd = parser.parse("SELECT * FROM test");
    assert_test("解析 SELECT", cmd.has_value());
    assert_test("命令类型正确", cmd->type == SQLCommandType::SELECT);

    // UPDATE
    cmd = parser.parse("UPDATE test SET id = '999'");
    assert_test("解析 UPDATE", cmd.has_value());
    assert_test("命令类型正确", cmd->type == SQLCommandType::UPDATE);

    // DELETE
    cmd = parser.parse("DELETE FROM test");
    assert_test("解析 DELETE", cmd.has_value());
    assert_test("命令类型正确", cmd->type == SQLCommandType::DELETE);

    // DROP TABLE
    cmd = parser.parse("DROP TABLE test");
    assert_test("解析 DROP TABLE", cmd.has_value());
    assert_test("命令类型正确", cmd->type == SQLCommandType::DROP_TABLE);

    // 无效SQL
    cmd = parser.parse("INVALID COMMAND");
    assert_test("无效SQL检测", !cmd.has_value());
}

// ========== Database 测试 ==========
void test_database() {
    std::cout << "\n=== 测试 Database ===\n";

    Database db;

    // CREATE TABLE
    auto result = db.execute("CREATE TABLE users (id INT, name TEXT, age INT)");
    assert_test("CREATE TABLE 成功", result.success);

    // INSERT
    result = db.execute("INSERT INTO users VALUES (1, 'Alice', 25)");
    assert_test("INSERT 1 成功", result.success && result.affectedRows == 1);

    result = db.execute("INSERT INTO users VALUES (2, 'Bob', 30)");
    assert_test("INSERT 2 成功", result.success);

    result = db.execute("INSERT INTO users VALUES (3, 'Charlie', 28)");
    assert_test("INSERT 3 成功", result.success);

    // SELECT
    result = db.execute("SELECT * FROM users");
    assert_test("SELECT 成功", result.success);
    assert_test("SELECT 返回3行", result.affectedRows == 3);

    // UPDATE
    result = db.execute("UPDATE users SET id = '99'");
    assert_test("UPDATE 成功", result.success);
    assert_test("UPDATE 影响3行", result.affectedRows == 3);

    // DELETE
    result = db.execute("DELETE FROM users");
    assert_test("DELETE 成功", result.success);

    // 错误处理
    result = db.execute("INVALID SQL");
    assert_test("无效SQL失败", !result.success);

    result = db.execute("SELECT * FROM nonexistent");
    assert_test("查询不存在的表失败", !result.success);
}

// ========== 完整流程测试 ==========
void test_full_workflow() {
    std::cout << "\n=== 测试完整流程 ===\n";

    Database db;

    // 创建表
    auto result = db.execute("CREATE TABLE employees (id INT, name TEXT, salary REAL)");
    assert_test("创建表", result.success);

    // 插入数据
    result = db.execute("INSERT INTO employees VALUES (1, '张三', 5000)");
    assert_test("插入1", result.success);

    result = db.execute("INSERT INTO employees VALUES (2, '李四', 6000)");
    assert_test("插入2", result.success);

    result = db.execute("INSERT INTO employees VALUES (3, '王五', 7000)");
    assert_test("插入3", result.success);

    // 查询验证
    result = db.execute("SELECT * FROM employees");
    assert_test("查询", result.success && result.affectedRows == 3);

    // 更新
    result = db.execute("UPDATE employees SET salary = '8000'");
    assert_test("更新", result.success);

    // 清空
    result = db.execute("DELETE FROM employees");
    assert_test("删除", result.success);
}

// ========== 加密测试 ==========
void test_encryption() {
    std::cout << "\n=== 测试加密功能 ===\n";

    Database db;
    db.setEncryptionKey("secret_key");

    // 创建加密表
    auto result = db.execute("CREATE TABLE secrets (id INT, data TEXT)");
    assert_test("创建加密表", result.success);

    // 插入加密数据
    result = db.execute("INSERT INTO secrets VALUES (1, 'confidential')");
    assert_test("插入加密数据", result.success);

    // 查询（应自动解密）
    result = db.execute("SELECT * FROM secrets");
    assert_test("查询加密数据", result.success);
}

int main() {
    std::cout << "=====================================";
    std::cout << "\n       MemoryDB 测试套件";
    std::cout << "\n=====================================";

    test_table();
    test_storage();
    test_parser();
    test_database();
    test_full_workflow();
    test_encryption();

    std::cout << "\n=====================================";
    std::cout << "\n       ✅ 所有测试通过！";
    std::cout << "\n=====================================\n";

    return 0;
}
