#!/usr/bin/env python3
"""
MemoryDB HTTP API Python 客户端示例

前提条件：
1. HTTP 服务已启动：./build/httpd.exe --port 8080
2. 安装 requests 库：pip install requests
"""

import requests
import json
from typing import Dict, Any


class MemoryDBClient:
    """MemoryDB HTTP API 客户端"""

    def __init__(self, host: str = "127.0.0.1", port: int = 8080):
        self.base_url = f"http://{host}:{port}"
        self.session = requests.Session()

    def execute(self, sql: str) -> Dict[str, Any]:
        """
        执行 SQL 语句

        Args:
            sql: SQL 语句

        Returns:
            响应 JSON 字典，包含：
            - success: bool
            - message: str
            - affectedRows: int
            - columns: List[str]
            - rows: List[List[str]]
        """
        payload = {"sql": sql}
        try:
            response = self.session.post(
                f"{self.base_url}/query",
                headers={"Content-Type": "application/json"},
                data=json.dumps(payload)
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

    def create_table(self, name: str, columns: Dict[str, str]) -> Dict[str, Any]:
        """创建表"""
        cols = ", ".join([f"{k} {v}" for k, v in columns.items()])
        sql = f"CREATE TABLE {name} ({cols})"
        return self.execute(sql)

    def drop_table(self, name: str) -> Dict[str, Any]:
        """删除表"""
        sql = f"DROP TABLE {name}"
        return self.execute(sql)

    def insert(self, table: str, values: list) -> Dict[str, Any]:
        """插入数据"""
        vals = ", ".join([f"'{v}'" if isinstance(v, str) else str(v) for v in values])
        sql = f"INSERT INTO {table} VALUES ({vals})"
        return self.execute(sql)

    def select(self, table: str) -> Dict[str, Any]:
        """查询数据"""
        sql = f"SELECT * FROM {table}"
        return self.execute(sql)

    def update(self, table: str, column: str, value: str) -> Dict[str, Any]:
        """更新数据"""
        sql = f"UPDATE {table} SET {column}='{value}'"
        return self.execute(sql)

    def delete(self, table: str) -> Dict[str, Any]:
        """删除数据"""
        sql = f"DELETE FROM {table}"
        return self.execute(sql)

    def close(self):
        """关闭连接"""
        self.session.close()


def main():
    """示例：完整的 CRUD 操作"""
    print("MemoryDB Python 客户端示例\n")
    print("=" * 50)

    # 创建客户端
    client = MemoryDBClient(host="127.0.0.1", port=8080)

    try:
        # 1. 创建表
        print("\n1. 创建表 users")
        result = client.create_table("users", {
            "id": "INT",
            "name": "TEXT",
            "age": "INT"
        })
        print(f"   Success: {result['success']}")
        print(f"   Message: {result['message']}")

        # 2. 插入数据
        print("\n2. 插入数据")
        client.insert("users", [1, "Alice", 25])
        print("   插入: (1, 'Alice', 25)")
        client.insert("users", [2, "Bob", 30])
        print("   插入: (2, 'Bob', 30)")
        client.insert("users", [3, "Charlie", 28])
        print("   插入: (3, 'Charlie', 28)")

        # 3. 查询数据
        print("\n3. 查询数据")
        result = client.select("users")
        print(f"   Success: {result['success']}")
        print(f"   Affected Rows: {result['affectedRows']}")
        print(f"   Columns: {result['columns']}")
        print("   Rows:")
        for row in result['rows']:
            print(f"     {row}")

        # 4. 更新数据
        print("\n4. 更新数据")
        result = client.update("users", "age", "26")
        print(f"   Success: {result['success']}")
        print(f"   Message: {result['message']}")
        print(f"   Affected Rows: {result['affectedRows']}")

        # 5. 再次查询验证更新
        print("\n5. 查询更新后的数据")
        result = client.select("users")
        for row in result['rows']:
            print(f"     {row}")

        # 6. 删除数据
        print("\n6. 删除数据")
        result = client.delete("users")
        print(f"   Success: {result['success']}")
        print(f"   Message: {result['message']}")
        print(f"   Affected Rows: {result['affectedRows']}")

        # 7. 清理
        print("\n7. 删除表")
        result = client.drop_table("users")
        print(f"   Success: {result['success']}")
        print(f"   Message: {result['message']}")

    finally:
        client.close()

    print("\n" + "=" * 50)
    print("示例完成")


if __name__ == "__main__":
    # 检查是否可以连接到服务器
    try:
        requests.get("http://127.0.0.1:8080/query", timeout=1)
    except:
        print("错误: 无法连接到 HTTP 服务器")
        print("请先启动服务器: ./build/httpd.exe --port 8080")
        exit(1)

    main()
