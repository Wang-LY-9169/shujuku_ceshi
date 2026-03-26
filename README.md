# MemoryDB

> 一个类似 SQLite 的轻量级内存数据库，C++17 实现，支持基础 SQL 操作、CLI 方式使用和本地 HTTP API 调用。

---

## 目录

- [简介](#简介)
- [特性](#特性)
- [项目结构](#项目结构)
- [架构说明](#架构说明)
- [功能说明](#功能说明)
- [编译与运行](#编译与运行)
- [HTTP API 使用说明](#http-api-使用说明)
- [使用示例](#使用示例)
- [当前限制](#当前限制)
- [后续可完善方向](#后续可完善方向)
- [注意事项](#注意事项)

---

## 简介

MemoryDB 是一个轻量级的内存数据库，采用 C++17 标准实现，提供了类似 SQLite 的基础 SQL 功能。项目适合用于学习数据库原理、原型验证和功能演示。

**当前版本**：v1.0
**开发语言**：C++17
**编译器**：g++ 14.2.0 (MinGW-W64 x86_64)
**构建系统**：CMake 3.31.5+

---

## 特性

- ✅ 支持 CREATE TABLE / DROP TABLE
- ✅ 支持 INSERT / SELECT / UPDATE / DELETE
- ✅ 支持多表管理
- ✅ 支持数据加密（XOR 实现，可扩展）
- ✅ 支持交互式 CLI 方式使用
- ✅ 支持本地 HTTP API 调用
- ✅ 支持 POST /query 接口
- ✅ 返回统一的 JSON 格式结果

---

## 项目结构

```
memorydb/
├── include/              # 头文件目录
│   ├── database.h       # Database 类 - 核心调度器
│   ├── sql_parser.h    # SQLParser - SQL 解析
│   ├── table.h         # Table - 数据表
│   ├── storage.h       # StorageEngine - 多表管理
│   ├── encryption.h    # CryptoEngine - 加密模块
│   ├── api_serializer.h  # ApiSerializer - JSON 序列化
│   └── http_server.h   # HttpServer - HTTP 服务
│
├── src/                 # 源文件目录
│   ├── main.cpp        # CLI 入口
│   ├── httpd_main.cpp  # HTTP 服务入口
│   ├── database.cpp    # Database 实现
│   ├── sql_parser.cpp  # SQLParser 实现
│   ├── table.cpp       # Table 实现
│   ├── storage.cpp     # StorageEngine 实现
│   ├── encryption.cpp  # CryptoEngine 实现
│   ├── api_serializer.cpp  # ApiSerializer 实现
│   └── http_server.cpp # HttpServer 实现
│
├── tests/               # 测试目录
│   ├── test_main.cpp   # 核心测试套件
│   └── test_api.cpp    # HTTP API 测试套件
│
├── examples/            # 示例代码
│   └── python_client.py # Python HTTP 客户端示例
│
├── build/               # 编译输出目录
│   ├── memorydb.exe    # CLI 程序
│   ├── httpd.exe       # HTTP 服务
│   ├── test_all.exe    # 核心测试
│   └── test_api.exe    # API 测试
│
├── CMakeLists.txt       # CMake 配置
└── README.md           # 本文件
```

**关键文件职责：**

- `database.h/cpp`：Database 类，核心调度器，负责 SQL 执行调度和结果统一
- `sql_parser.h/cpp`：SQLParser 类，负责将 SQL 字符串解析为结构化命令
- `table.h/cpp`：Table 类，负责单个表的数据存储和操作
- `storage.h/cpp`：StorageEngine 类，负责管理多个数据表
- `encryption.h/cpp`：CryptoEngine 类，负责数据加密/解密（当前为 XOR 实现）
- `api_serializer.h/cpp`：ApiSerializer 类，负责将 QueryResult 序列化为 JSON
- `http_server.h/cpp`：HttpServer 类，负责 HTTP 服务和请求处理
- `main.cpp`：CLI 入口，处理用户交互
- `httpd_main.cpp`：HTTP 服务入口，启动监听服务

---

## 架构说明

### 模块关系

```
用户输入 → Database::execute() → SQLParser::parse() → SQLCommand
                                ↓
                         StorageEngine::操作表
                                ↓
                         Table::CRUD操作
                                ↓
                         QueryResult返回
                                ↓
                         ApiSerializer::serialize() → JSON
```

**CLI 路径：**
用户 → CLI → Database::execute() → QueryResult → CLI 展示

**HTTP API 路径：**
HTTP 请求 → HttpServer → Database::execute() → QueryResult → ApiSerializer → JSON 响应

### 核心模块说明

- **Database**：核心调度器，对外提供统一的 `execute()` 接口，内部协调 SQLParser、StorageEngine 和其他模块
- **SQLParser**：负责解析 SQL 字符串，生成 `SQLCommand` 结构（包含命令类型、表名、列、值等）
- **StorageEngine**：使用 `std::map` 管理所有数据表，提供表的创建、删除、获取等功能
- **Table**：单个数据表，使用 `std::vector` 存储数据行，提供行的增删改查功能
- **CryptoEngine**：数据加密引擎，当前实现简单的 XOR 加密，可通过 `setCryptoEngine()` 关联到 Table
- **ApiSerializer**：JSON 序列化模块，负责将 `QueryResult` 转换为 JSON 字符串
- **HttpServer**：HTTP 服务模块，监听 127.0.0.1:8080，处理 POST /query 请求

---

## 功能说明

### 支持的 SQL 能力

| 操作 | 语法 | 说明 |
|------|------|------|
| CREATE TABLE | `CREATE TABLE name (col1 type1, col2 type2, ...)` | 创建数据表，支持 INT、TEXT 类型 |
| DROP TABLE | `DROP TABLE name` | 删除数据表 |
| INSERT | `INSERT INTO name VALUES (val1, val2, ...)` | 插入数据行 |
| SELECT | `SELECT * FROM name` | 查询所有数据（WHERE 条件当前未实现） |
| UPDATE | `UPDATE name SET col=val` | 更新数据（WHERE 条件当前未实现） |
| DELETE | `DELETE FROM name` | 删除数据（WHERE 条件当前未实现） |

### 当前接口支持

| 程序 | 接口 | 说明 |
|------|------|------|
| CLI | 标准输入输出 | 交互式命令行界面 |
| HTTP API | POST /query | JSON 请求/响应 |

### 错误处理

| 情况 | HTTP 状态码 | 响应内容 |
|------|-------------|----------|
| SQL 语法错误 | 200 | `{"success": false, "message": "..."}` |
| 表不存在 | 200 | `{"success": false, "message": "..."}` |
| 缺少 sql 字段 | 400 | `{"success": false, "errorCode": 400, "message": "Missing 'sql' field..."}` |
| 错误路径 | 404 | `{"success": false, "errorCode": 404, "message": "Not Found"}` |

---

## 编译与运行

### 环境要求

| 组件 | 要求 |
|------|------|
| 操作系统 | Windows / Linux / macOS |
| 编译器 | g++ 9+ / Clang 10+ / MSVC 2019+ |
| C++ 标准 | C++17 或更高 |
| CMake | 3.15+ (可选) |

### 编译方式

**方式一：使用 CMake**
```bash
cd build
cmake ..
cmake --build .
```

**方式二：直接使用 g++**
```bash
# 编译 CLI
g++ -std=c++17 -Wall -O2 -I ../include \
    ../src/database.cpp ../src/sql_parser.cpp ../src/table.cpp \
    ../src/encryption.cpp ../src/storage.cpp ../src/api_serializer.cpp \
    ../src/main.cpp -o memorydb.exe -lws2_32

# 编译 HTTP 服务
g++ -std=c++17 -Wall -O2 -I ../include \
    ../src/database.cpp ../src/sql_parser.cpp ../src/table.cpp \
    ../src/encryption.cpp ../src/storage.cpp ../src/api_serializer.cpp \
    ../src/http_server.cpp ../src/httpd_main.cpp -o httpd.exe -lws2_32

# 编译测试
g++ -std=c++17 -Wall -O2 -I ../include \
    ../src/database.cpp ../src/sql_parser.cpp ../src/table.cpp \
    ../src/encryption.cpp ../src/storage.cpp ../src/api_serializer.cpp \
    ../tests/test_main.cpp -o test_all.exe

g++ -std=c++17 -Wall -O2 -I ../include \
    ../src/database.cpp ../src/sql_parser.cpp ../src/table.cpp \
    ../src/encryption.cpp ../src/storage.cpp ../src/api_serializer.cpp \
    ../src/http_server.cpp ../tests/test_api.cpp -o test_api.exe -lws2_32
```

### 运行 CLI

```bash
cd build
./memorydb.exe
```

### 启动 HTTP 服务

```bash
# 使用默认端口 8080
cd build
./httpd.exe

# 指定端口
./httpd.exe --port 9000

# 查看帮助
./httpd.exe --help
```

### 运行测试

```bash
# 核心测试（无需 HTTP 服务）
cd build
./test_all.exe

# HTTP API 测试（需要先启动 httpd）
cd build
./test_api.exe
```

---

## HTTP API 使用说明

### 服务监听地址

服务监听 `127.0.0.1:8080`（可通过 `--port` 参数修改），仅支持本地调用。

### POST /query 接口

**URL**：`POST http://127.0.0.1:8080/query`

**请求头**：
```
Content-Type: application/json
```

**请求格式**：
```json
{
  "sql": "SELECT * FROM users"
}
```

**响应格式**：
```json
{
  "success": true,
  "message": "查询成功，共 2 行",
  "affectedRows": 2,
  "columns": ["id", "name", "age"],
  "rows": [
    ["1", "Alice", "25"],
    ["2", "Bob", "30"]
  ]
}
```

**响应字段说明**：

| 字段 | 类型 | 说明 |
|------|------|------|
| success | bool | 操作是否成功 |
| message | string | 结果描述信息 |
| affectedRows | int | 影响的行数（SELECT 时为查询结果行数） |
| columns | array | 列名数组（SELECT 时有内容） |
| rows | array | 数据行数组，每行是一个字符串数组（SELECT 时有内容） |

### curl 示例

**创建表：**
```bash
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"CREATE TABLE users (id INT, name TEXT, age INT)\"}"
```

**插入数据：**
```bash
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"INSERT INTO users VALUES (1, 'Alice', 25)\"}"
```

**查询数据：**
```bash
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"SELECT * FROM users\"}"
```

**更新数据：**
```bash
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"UPDATE users SET age=26\"}"
```

**删除数据：**
```bash
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"DELETE FROM users\"}"
```

**删除表：**
```bash
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"DROP TABLE users\"}"
```

### Python requests 示例

首先安装 requests 库：
```bash
pip install requests
```

**Python 示例代码：**
```python
import requests

# 创建数据库连接
class MemoryDBClient:
    def __init__(self, host="127.0.0.1", port=8080):
        self.base_url = f"http://{host}:{port}"
        self.session = requests.Session()

    def execute(self, sql):
        payload = {"sql": sql}
        try:
            response = self.session.post(
                f"{self.base_url}/query",
                json=payload
            )
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            return {
                "success": False,
                "message": str(e),
                "affectedRows": 0,
                "columns": [],
                "rows": []
            }

# 使用示例
db = MemoryDBClient()

# 创建表
result = db.execute("CREATE TABLE users (id INT, name TEXT, age INT)")
print(result)

# 插入数据
db.execute("INSERT INTO users VALUES (1, 'Alice', 25)")
db.execute("INSERT INTO users VALUES (2, 'Bob', 30)")

# 查询数据
result = db.execute("SELECT * FROM users")
print(result["rows"])  # 打印所有数据

# 更新数据
result = db.execute("UPDATE users SET age=26")

# 删除数据
result = db.execute("DELETE FROM users")

# 删除表
result = db.execute("DROP TABLE users")
```

完整的 Python 示例代码请参考 `examples/python_client.py`。

---

## 使用示例

### CLI 交互示例

```
=====================================
       MemoryDB v1.0
       轻量级内存数据库
=====================================


=== MemoryDB 帮助 ===

SQL 命令:
  CREATE TABLE name (col1 type1, col2 type2, ...)
  DROP TABLE name
  INSERT INTO name VALUES (val1, val2, ...)
  SELECT * FROM name
  UPDATE name SET col=val
  DELETE FROM name

系统命令:
  help      - 显示帮助
  tables    - 列出所有表
  exit      - 退出程序

memorydb> CREATE TABLE users (id INT, name TEXT, age INT)
OK

memorydb> INSERT INTO users VALUES (1, 'Alice', 25)
OK (1 行受影响)

memorydb> INSERT INTO users VALUES (2, 'Bob', 30)
OK (1 行受影响)

memorydb> SELECT * FROM users
----------------------
| id | name  | age |
----------------------
| 1  | Alice | 25  |
| 2  | Bob   | 30  |
----------------------
2 行

memorydb> UPDATE users SET age=26
OK (2 行受影响)

memorydb> DELETE FROM users
OK (2 行受影响)

memorydb> tables
表列表 (1 个):
  users

memorydb> DROP TABLE users
OK

memorydb> exit
再见！
```

### HTTP 调用示例

**完整的 CRUD 流程：**
```bash
# 1. 创建表
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"CREATE TABLE products (id INT, name TEXT, price INT)\"}"

# 2. 插入多条数据
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"INSERT INTO products VALUES (1, 'Apple', 5)\"}"

curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"INSERT INTO products VALUES (2, 'Banana', 3)\"}"

# 3. 查询所有数据
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"SELECT * FROM products\"}"

# 4. 更新数据
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"UPDATE products SET price=4\"}"

# 5. 删除所有数据
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"DELETE FROM products\"}"

# 6. 删除表
curl -X POST http://127.0.0.1:8080/query \
  -H "Content-Type: application/json" \
  -d "{\"sql\": \"DROP TABLE products\"}"
```

---

## 当前限制

- **内存存储**：数据存储在内存中，重启或程序退出后数据丢失
- **本地访问限制**：HTTP 服务仅监听 `127.0.0.1`，不支持公网访问
- **WHERE 条件**：当前未实现复杂的 WHERE 条件表达式
- **索引**：当前未实现索引机制
- **事务**：当前未实现事务机制
- **JOIN 操作**：当前未实现表连接查询
- **数据类型**：当前仅支持 INT、TEXT 类型
- **加密算法**：当前仅实现简单的 XOR 加密，未替换为正式算法
- **持久化**：当前未实现数据持久化到文件

---

## 后续可完善方向

基于当前工程现状，以下是可以考虑的完善方向（实际实现需谨慎评估）：

**数据库功能：**
- 数据持久化：将数据保存到文件，支持重启后恢复
- WHERE 条件增强：支持复杂的条件表达式和运算符
- 索引机制：为常用查询列建立索引，提升查询性能
- 事务支持：实现原子性的多操作提交和回滚
- JOIN 操作：支持多表连接查询
- 数据类型扩展：支持更多数据类型（REAL、BLOB、DATE 等）

**加密模块：**
- AES 加密替换 XOR：实现更安全的加密算法
- 密钥管理支持：支持密钥轮换和版本管理

**API 层：**
- 连接池：支持多个并发连接
- 批量操作：支持一次执行多条 SQL
- 预处理语句：支持参数化查询
- 认证机制：支持基本的用户名密码认证

**后续开发时请务必保持现有模块边界，避免过度耦合。**

---

## 注意事项

- **适用场景**：当前项目适合学习数据库原理、原型验证和功能演示
- **不适合生产**：由于缺乏持久化、完整的并发控制、正式的加密算法等，不建议直接作为生产数据库使用
- **模块边界**：后续开发时应保持现有模块边界，不要破坏 Database、SQLParser、StorageEngine、Table、CryptoEngine、ApiSerializer、HttpServer 等模块的职责划分
- **测试覆盖**：每次修改后请确保核心测试（51 个）和 API 测试（11 个）全部通过
- **编译兼容性**：保持 C++17 标准和 MinGW 兼容性，避免使用非标准库或特性

---

## 许可证

MIT License
