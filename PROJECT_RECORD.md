# MemoryDB 项目开发记录

## 项目信息

- **项目名称**: MemoryDB (内存数据库)
- **开发日期**: 2026-03-06
- **项目语言**: C++17
- **构建系统**: CMake
- **编译器**: g++ 14.2.0 (winlibs-mingw-w64)

---

## 项目概述

一个类似 SQLite 的轻量级内存数据库，支持：
- SQL 语句操作数据表（CREATE、DROP、INSERT、SELECT、UPDATE、DELETE）
- 表内数据增删改查
- 数据加密保护机制（XOR）
- 交互式 CLI 终端

---

## 开发步骤记录

### Step 1: 实现 Table 类

**时间**: 第一步
**目标**: 实现基础表管理

**关键节点**:
1. 创建 `table.h` - 定义 Table 类和 Row 类型
2. 创建 `table.cpp` - 实现表操作
3. 创建 `test_table.cpp` - 测试代码

**实现功能**:
- `Table(const std::string& name)` - 构造函数
- `setSchema()` - 设置表结构
- `insertRow()` - 插入数据行
- `selectRows()` - 查询所有行
- `updateRows()` - 更新行
- `deleteRows()` - 删除所有行
- `print()` - 打印表内容

**测试结果**: ✅ 全部通过

---

### Step 2: 实现 StorageEngine

**时间**: 第二步
**目标**: 管理多个表

**关键节点**:
1. 创建 `storage.h` - 定义存储引擎
2. 创建 `storage.cpp` - 实现表管理
3. 创建 `test_storage.cpp` - 测试代码

**实现功能**:
- `createTable()` - 创建表
- `dropTable()` - 删除表
- `getTable()` - 获取表
- `listTables()` - 列出所有表
- `tableExists()` - 检查表是否存在

**数据结构**:
```cpp
std::map<std::string, std::unique_ptr<Table>> tables_;
```

**测试结果**: ✅ 全部通过

---

### Step 3: 实现 SQLParser

**时间**: 第三步
**目标**: 解析 SQL 字符串

**关键节点**:
1. 创建 `sql_parser.h` - 定义解析器接口
2. 创建 `sql_parser.cpp` - 实现解析逻辑
3. 创建 `test_parser.cpp` - 测试代码

**支持语句**:
- `CREATE TABLE name (col1 type1, ...)`
- `DROP TABLE name`
- `INSERT INTO name VALUES (val1, ...)`
- `SELECT * FROM name [WHERE condition]`
- `UPDATE name SET col=val [WHERE condition]`
- `DELETE FROM name [WHERE condition]`

**SQLCommand 结构**:
```cpp
struct SQLCommand {
    SQLCommandType type;
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<std::string> values;
    std::string condition;
};
```

**测试结果**: ✅ 全部通过

---

### Step 4: 实现 Database 类

**时间**: 第四步
**目标**: 核心调度器，整合所有模块

**关键节点**:
1. 创建 `database.h` - 定义 Database 类
2. 创建 `database.cpp` - 实现调度逻辑
3. 创建 `test_database.cpp` - 集成测试

**执行流程**:
```
SQL 输入
   ↓
SQLParser 解析
   ↓
SQLCommand
   ↓
StorageEngine 操作
   ↓
Table 执行
   ↓
QueryResult 返回
```

**实现功能**:
- `execute()` - 执行 SQL 语句
- `executeCreateTable()` - 执行 CREATE
- `executeDropTable()` - 执行 DROP
- `executeInsert()` - 执行 INSERT
- `executeSelect()` - 执行 SELECT
- `executeUpdate()` - 执行 UPDATE
- `executeDelete()` - 执行 DELETE

**测试结果**: ✅ 全部通过

---

### Step 5: 实现 CryptoEngine

**时间**: 第五步
**目标**: 数据加密保护

**关键节点**:
1. 创建 `encryption.h` - 定义加密引擎
2. 创建 `encryption.cpp` - 实现 XOR 加密
3. 修改 `table.h/cpp` - 支持加密
4. 修改 `database.h/cpp` - 集成加密
5. 创建 `test_encryption.cpp` - 测试加密

**实现功能**:
- `CryptoEngine::encrypt()` - 加密字符串
- `CryptoEngine::decrypt()` - 解密字符串
- `Database::setEncryptionKey()` - 设置加密密钥
- Table 插入时自动加密
- Table 查询时自动解密

**加密算法**: XOR（临时方案）

**数据流向**:
```
用户输入数据
   ↓
加密 (如果启用)
   ↓
存储到 Table (加密状态)
   ↓
查询时解密
   ↓
返回明文数据
```

**测试结果**: ✅ 全部通过

---

### Step 6: 实现 CLI 终端

**时间**: 第六步
**目标**: 交互式命令行界面

**关键节点**:
1. 创建 `main.cpp` - 实现 CLI

**实现功能**:
- 启动界面显示横幅和帮助
- `memorydb>` 提示符
- `help` - 显示帮助
- `tables` - 列出所有表
- `exit/quit` - 退出程序
- 直接输入 SQL 执行

**CLI 测试**:
```
memorydb> CREATE TABLE users (id INT, name TEXT, age INT)
OK

memorydb> INSERT INTO users VALUES (1, 'Alice', 25)
OK (1 行受影响)

memorydb> SELECT * FROM users
| id | name  | age |
| 1  | Alice | 25  |

memorydb> exit
再见！
```

**测试结果**: ✅ 全部通过

---

### Step 7: 编写测试套件

**时间**: 第七步
**目标**: 统一测试入口

**关键节点**:
1. 创建 `test_main.cpp` - 统一测试套件

**测试覆盖**:
- Table 基础操作（9个断言）
- StorageEngine 多表管理（9个断言）
- SQLParser 解析（12个断言）
- Database 集成功能（9个断言）
- 完整 CRUD 流程（6个断言）
- 加密功能（3个断言）

**测试结果**: ✅ 48/48 断言全部通过

---

## 最终项目结构

```
memorydb/
├── include/              # 头文件目录
│   ├── table.h         # Table 类定义
│   ├── storage.h       # StorageEngine 定义
│   ├── sql_parser.h    # SQLParser 定义
│   ├── database.h      # Database 定义
│   └── encryption.h    # CryptoEngine 定义
│
├── src/                 # 源文件目录
│   ├── main.cpp        # CLI 入口
│   ├── table.cpp       # Table 实现
│   ├── storage.cpp     # StorageEngine 实现
│   ├── sql_parser.cpp  # SQLParser 实现
│   ├── database.cpp    # Database 实现
│   └── encryption.cpp  # CryptoEngine 实现
│
├── tests/               # 测试目录
│   └── test_main.cpp  # 统一测试套件
│
├── build/               # 编译输出
│   ├── memorydb.exe    # 主程序 (480KB)
│   └── test_all.exe    # 测试程序 (518KB)
│
├── CMakeLists.txt       # CMake 配置
└── README.md            # 项目说明
```

---

## 环境配置记录

### 编译环境
- **操作系统**: Windows 11 (Build 10.0.26200)
- **Shell**: Git Bash / MSYS2 (MINGW64)
- **编译器**: g++ 14.2.0 (winlibs-x86_64-posix-seh-gcc-14.2.0-mingw-w64ucrt-12.0.0-r3)
- **CMake**: 3.31.5

### 编译命令

**编译主程序**:
```bash
cd build
g++ -std=c++17 -Wall -I ../include \
    ../src/table.cpp ../src/storage.cpp \
    ../src/sql_parser.cpp ../src/database.cpp \
    ../src/encryption.cpp ../src/main.cpp \
    -o memorydb.exe
```

**编译测试程序**:
```bash
cd build
g++ -std=c++17 -Wall -I ../include \
    ../src/table.cpp ../src/storage.cpp \
    ../src/sql_parser.cpp ../src/database.cpp \
    ../src/encryption.cpp ../tests/test_main.cpp \
    -o test_all.exe
```

---

## 使用说明

### 运行程序
```bash
cd build
./memorydb.exe
```

### 支持的 SQL 命令
| 命令 | 示例 | 说明 |
|--------|--------|------|
| CREATE TABLE | `CREATE TABLE users (id INT, name TEXT)` | 创建表 |
| DROP TABLE | `DROP TABLE users` | 删除表 |
| INSERT | `INSERT INTO users VALUES (1, 'Alice')` | 插入数据 |
| SELECT | `SELECT * FROM users` | 查询数据 |
| UPDATE | `UPDATE users SET name = 'Bob'` | 更新数据 |
| DELETE | `DELETE FROM users` | 删除数据 |

### CLI 命令
| 命令 | 说明 |
|--------|------|
| `help` 或 `?` | 显示帮助 |
| `tables` | 列出所有表 |
| `exit` 或 `quit` | 退出程序 |

### 运行测试
```bash
cd build
./test_all.exe
```

---

## 功能特性总结

### 已实现功能 ✅
- ✅ CREATE TABLE - 创建数据表
- ✅ DROP TABLE - 删除数据表
- ✅ INSERT - 插入数据
- ✅ SELECT - 查询数据
- ✅ UPDATE - 更新数据
- ✅ DELETE - 删除数据
- ✅ XOR 加密/解密
- ✅ 多表管理
- ✅ SQL 解析
- ✅ 错误处理
- ✅ 交互式 CLI

### 待扩展功能 🚧
- 🚧 WHERE 子句增强（支持复杂条件表达式）
- 🚧 AES 加密实现
- 🚧 数据持久化（文件存储）
- 🚧 B-tree 索引
- 🚧 事务机制
- 🚧 JOIN 操作

---

## 技术要点

### 数据类型定义
```cpp
using Row = std::vector<std::string>;

struct ColumnDef {
    std::string name;
    std::string type;
};
```

### 存储结构
```cpp
std::map<std::string, std::unique_ptr<Table>> tables_;
```

### 加密流程
```cpp
// 加密时
std::string encrypted = crypto_->encrypt(plaintext);
rows_.push_back(encrypted);

// 解密时
std::string decrypted = crypto_->decrypt(encrypted);
return decrypted;
```

---

## 故障排除记录

### 编译错误 - uint8_t 未声明
**问题**: 编译时报错 `uint8_t was not declared`
**解决**: 在头文件中添加 `#include <cstdint>`

### 编译错误 - 重复 namespace 声明
**问题**: `namespace MemoryDB {` 重复声明
**解决**: 移除重复的 namespace 声明，只保留一个

### 空间问题 - Claude 临时文件占用 22GB
**问题**: `C:\Users\10698\AppData\Local\Temp\claude` 占用 22GB
**解决**: 删除临时缓存目录

---

## 测试统计

| 测试模块 | 测试数量 | 通过 | 失败 |
|----------|----------|-------|--------|
| Table | 9 | 9 | 0 |
| StorageEngine | 9 | 9 | 0 |
| SQLParser | 12 | 12 | 0 |
| Database | 9 | 9 | 0 |
| 完整流程 | 6 | 6 | 0 |
| 加密功能 | 3 | 3 | 0 |
| **总计** | **48** | **48** | **0** |

---

## 文件清单

### 头文件 (include/)
- `table.h` - Table 类定义
- `storage.h` - StorageEngine 定义
- `sql_parser.h` - SQLParser 定义
- `database.h` - Database 定义
- `encryption.h` - CryptoEngine 定义

### 源文件 (src/)
- `main.cpp` - CLI 入口
- `table.cpp` - Table 实现
- `storage.cpp` - StorageEngine 实现
- `sql_parser.cpp` - SQLParser 实现
- `database.cpp` - Database 实现
- `encryption.cpp` - CryptoEngine 实现

### 测试文件 (tests/)
- `test_main.cpp` - 统一测试套件

### 可执行文件 (build/)
- `memorydb.exe` - 主程序
- `test_all.exe` - 测试套件

---

## 查看记录的方式

### 方式一：直接打开
```bash
# Windows 记事本
notepad E:\Claudecodeproject\shujuku\PROJECT_RECORD.md

# 或 VS Code
code E:\Claudecodeproject\shujuku\PROJECT_RECORD.md
```

### 方式二：项目目录中查看
```bash
cd E:\Claudecodeproject\shujuku
cat PROJECT_RECORD.md
```

### 方式三：通过 CLI 查看（如果实现）
```sql
memorydb> SELECT content FROM records
```

---

## 项目完成状态

| 阶段 | 状态 | 完成度 |
|--------|--------|----------|
| Step 1: Table | ✅ 完成 | 100% |
| Step 2: StorageEngine | ✅ 完成 | 100% |
| Step 3: SQLParser | ✅ 完成 | 100% |
| Step 4: Database | ✅ 完成 | 100% |
| Step 5: CryptoEngine | ✅ 完成 | 100% |
| Step 6: CLI 终端 | ✅ 完成 | 100% |
| Step 7: 测试套件 | ✅ 完成 | 100% |

**总体完成度**: 100% ✅

---

## 关键代码片段

### SQL 解析示例
```cpp
if (startsWith(upperSql, "CREATE TABLE")) {
    return parseCreateTable(trimmedSql);
} else if (startsWith(upperSql, "INSERT INTO")) {
    return parseInsert(trimmedSql);
}
// ...
```

### 加密示例
```cpp
std::string CryptoEngine::xorEncryptDecrypt(const std::string& data) {
    std::string result = data;
    size_t keyLen = key_.length();

    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = data[i] ^ key_[i % keyLen];
    }
    return result;
}
```

### Database 执行流程
```cpp
QueryResult Database::execute(const std::string& sql) {
    auto cmd = parser_.parse(sql);
    if (!cmd) {
        return QueryResult{false, "SQL语法错误", 0};
    }

    switch (cmd->type) {
        case SQLCommandType::CREATE_TABLE:
            return executeCreateTable(*cmd);
        case SQLCommandType::INSERT:
            return executeInsert(*cmd);
        // ...
    }
}
```

---

**记录创建时间**: 2026-03-06
**记录版本**: v1.0
**记录状态**: 完整
