#ifndef MEMORYDB_HTTP_SERVER_H
#define MEMORYDB_HTTP_SERVER_H

#include "database.h"
#include "api_serializer.h"
#include <string>
#include <functional>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
// 只在 MSVC 下使用 #pragma comment
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#endif

namespace MemoryDB {

class HttpServer {
public:
    HttpServer(int port);
    ~HttpServer();

    // 启动 HTTP 服务（阻塞）
    bool start();

    // 停止 HTTP 服务
    void stop();

private:
    int port_;
    bool running_;
#ifdef _WIN32
    WSADATA wsaData_;
#endif

    // 处理单个客户端连接
    void handleClient(SOCKET clientSocket, Database& db);

    // 解析 HTTP 请求
    struct HttpRequest {
        std::string method;
        std::string path;
        std::string body;
        std::string contentType;
    };
    HttpRequest parseRequest(const std::string& requestStr);

    // 构造 HTTP 响应
    std::string buildResponse(int statusCode, const std::string& body, const std::string& contentType = "application/json");

    // 验证 JSON 格式是否合法
    bool isValidJson(const std::string& str);

    // 解析 JSON 请求体（简单实现）
    std::string extractSqlFromJson(const std::string& jsonBody);
};

} // namespace MemoryDB

#endif // MEMORYDB_HTTP_SERVER_H
