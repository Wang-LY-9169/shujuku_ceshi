#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#endif

// 简单的 HTTP 客户端
class HttpClient {
public:
    HttpClient(const std::string& host, int port) : host_(host), port_(port) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }

    ~HttpClient() {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    std::string post(const std::string& path, const std::string& jsonBody) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            return "";
        }

#ifdef _WIN32
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port_);
        serverAddr.sin_addr.s_addr = inet_addr(host_.c_str());
#else
        struct hostent* host = gethostbyname(host_.c_str());
        if (!host) {
            close(sock);
            return "";
        }

        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port_);
        memcpy(&serverAddr.sin_addr, host->h_addr, host->h_length);
#endif

        if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            return "";
        }

        std::ostringstream request;
        request << "POST " << path << " HTTP/1.1\r\n";
        request << "Host: " << host_ << "\r\n";
        request << "Content-Type: application/json\r\n";
        request << "Content-Length: " << jsonBody.length() << "\r\n";
        request << "Connection: close\r\n";
        request << "\r\n";
        request << jsonBody;

        std::string requestStr = request.str();
        send(sock, requestStr.c_str(), requestStr.length(), 0);

        char buffer[8192] = {0};
        std::string response;
        int bytesReceived;
        while ((bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            response.append(buffer, bytesReceived);
            memset(buffer, 0, sizeof(buffer));
        }

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif

        size_t headerEnd = response.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            return response.substr(headerEnd + 4);
        }
        return "";
    }

private:
    std::string host_;
    int port_;
};

void printTest(const std::string& name) {
    std::cout << "  " << name << " ... ";
}

void printPass() {
    std::cout << "✅\n";
}

void printFail() {
    std::cout << "❌\n";
}

// 全局测试计数
int passed = 0;
int failed = 0;

void test(bool condition) {
    if (condition) {
        printPass();
        passed++;
    } else {
        printFail();
        failed++;
    }
}

int main() {
    std::cout << "=====================================\n";
    std::cout << "       MemoryDB API 测试套件\n";
    std::cout << "=====================================\n\n";

    HttpClient client("127.0.0.1", 8080);

    std::cout << "=== HTTP API 测试 ===\n\n";

    // 测试 1: POST /query 执行 CREATE TABLE
    {
        printTest("1. POST /query CREATE TABLE");
        std::string response = client.post("/query", "{\"sql\": \"CREATE TABLE users (id INT, name TEXT, age INT)\"}");
        test(response.find("success\":true") != std::string::npos);
    }

    // 测试 2: POST /query 执行 INSERT
    {
        printTest("2. POST /query INSERT");
        std::string response = client.post("/query", "{\"sql\": \"INSERT INTO users VALUES (1, 'Alice', 25)\"}");
        test(response.find("success\":true") != std::string::npos &&
             response.find("affectedRows\":1") != std::string::npos);
    }

    {
        printTest("3. POST /query INSERT (second row)");
        std::string response = client.post("/query", "{\"sql\": \"INSERT INTO users VALUES (2, 'Bob', 30)\"}");
        test(response.find("success\":true") != std::string::npos &&
             response.find("affectedRows\":1") != std::string::npos);
    }

    // 测试 3: POST /query 执行 SELECT
    {
        printTest("4. POST /query SELECT");
        std::string response = client.post("/query", "{\"sql\": \"SELECT * FROM users\"}");
        test(response.find("success\":true") != std::string::npos &&
             response.find("affectedRows\":2") != std::string::npos &&
             response.find("columns") != std::string::npos &&
             response.find("rows") != std::string::npos &&
             response.find("Alice") != std::string::npos);
    }

    // 测试 4: POST /query 执行 UPDATE
    {
        printTest("5. POST /query UPDATE");
        std::string response = client.post("/query", "{\"sql\": \"UPDATE users SET age=26\"}");
        test(response.find("success\":true") != std::string::npos &&
             response.find("affectedRows\":2") != std::string::npos);
    }

    // 测试 5: POST /query 执行 DELETE
    {
        printTest("6. POST /query DELETE");
        std::string response = client.post("/query", "{\"sql\": \"DELETE FROM users\"}");
        test(response.find("success\":true") != std::string::npos &&
             response.find("affectedRows\":2") != std::string::npos);
    }

    // 测试 6: 缺少 sql 字段 -> HTTP 400
    {
        printTest("7. 缺少 sql 字段 -> HTTP 400");
        std::string response = client.post("/query", "{\"other\": \"value\"}");
        test(response.find("success\":false") != std::string::npos &&
             response.find("errorCode\":400") != std::string::npos &&
             response.find("Missing 'sql' field") != std::string::npos);
    }

    // 测试 7: 错误路径 -> HTTP 404
    {
        printTest("8. 错误路径 -> HTTP 404");
        std::string response = client.post("/notfound", "{\"sql\": \"SELECT 1\"}");
        test(response.find("success\":false") != std::string::npos &&
             response.find("errorCode\":404") != std::string::npos &&
             response.find("Not Found") != std::string::npos);
    }

    // 测试 8: 表不存在 -> HTTP 200 + success=false
    {
        printTest("9. 表不存在 -> HTTP 200 + success=false");
        std::string response = client.post("/query", "{\"sql\": \"SELECT * FROM nonexistent\"}");
        test(response.find("success\":false") != std::string::npos);
    }

    // 测试 9: SQL 语法错误 -> HTTP 200 + success=false
    {
        printTest("10. SQL 语法错误 -> HTTP 200 + success=false");
        std::string response = client.post("/query", "{\"sql\": \"INVALID SQL COMMAND\"}");
        test(response.find("success\":false") != std::string::npos);
    }

    // 清理
    {
        printTest("11. 清理 - DROP TABLE");
        std::string response = client.post("/query", "{\"sql\": \"DROP TABLE users\"}");
        test(response.find("success\":true") != std::string::npos);
    }

    std::cout << "\n=====================================\n";
    std::cout << "       ✅ 测试完成\n";
    std::cout << "=====================================\n";
    std::cout << "通过: " << passed << "\n";
    std::cout << "失败: " << failed << "\n\n";

    return (failed == 0) ? 0 : 1;
}
