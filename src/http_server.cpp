#include "http_server.h"
#include <sstream>
#include <cstring>
#include <iostream>

namespace MemoryDB {

HttpServer::HttpServer(int port) : port_(port), running_(false) {
#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &wsaData_);
#endif
}

HttpServer::~HttpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool HttpServer::start() {
#ifdef _WIN32
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }
#else
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
#endif

    // 设置 SO_REUSEADDR
#ifdef _WIN32
    char reuse = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#else
    int reuse = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(port_);

#ifdef _WIN32
    if (::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind to 127.0.0.1:" << port_ << std::endl;
        closesocket(serverSocket);
        return false;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Failed to listen" << std::endl;
        closesocket(serverSocket);
        return false;
    }
#else
    if (::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind to 127.0.0.1:" << port_ << std::endl;
        close(serverSocket);
        return false;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Failed to listen" << std::endl;
        close(serverSocket);
        return false;
    }
#endif

    running_ = true;
    std::cout << "HTTP Server started on http://127.0.0.1:" << port_ << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    Database db;

    while (running_) {
#ifdef _WIN32
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
#else
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
#endif
            if (running_) {
                std::cerr << "Failed to accept connection" << std::endl;
            }
            continue;
        }

        handleClient(clientSocket, db);

#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
    }

#ifdef _WIN32
    closesocket(serverSocket);
#else
    close(serverSocket);
#endif

    return true;
}

void HttpServer::stop() {
    running_ = false;
}

void HttpServer::handleClient(SOCKET clientSocket, Database& db) {
    char buffer[8192] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived <= 0) {
        return;
    }

    std::string requestStr(buffer, bytesReceived);
    HttpRequest request = parseRequest(requestStr);

    // 只处理 POST /query
    if (request.method != "POST" || request.path != "/query") {
        std::string response = buildResponse(404, ApiSerializer::error("Not Found", 404));
        send(clientSocket, response.c_str(), response.length(), 0);
        return;
    }

    // 检查 Content-Type
    if (request.contentType.find("application/json") == std::string::npos) {
        std::string response = buildResponse(400, ApiSerializer::error("Content-Type must be application/json", 400));
        send(clientSocket, response.c_str(), response.length(), 0);
        return;
    }

    // 简单验证：请求体必须以 { 开头，以 } 结尾
    std::string body = request.body;
    if (body.empty() || body[0] != '{' || body[body.length() - 1] != '}') {
        std::string response = buildResponse(400, ApiSerializer::error("Invalid JSON request body", 400));
        send(clientSocket, response.c_str(), response.length(), 0);
        return;
    }

    // 解析 SQL
    std::string sql = extractSqlFromJson(body);
    if (sql.empty()) {
        std::string response = buildResponse(400, ApiSerializer::error("Missing 'sql' field in request body", 400));
        send(clientSocket, response.c_str(), response.length(), 0);
        return;
    }

    // 执行 SQL
    QueryResult result = db.execute(sql);
    std::string jsonResult = ApiSerializer::serialize(result);
    std::string response = buildResponse(200, jsonResult);
    send(clientSocket, response.c_str(), response.length(), 0);
}

HttpServer::HttpRequest HttpServer::parseRequest(const std::string& requestStr) {
    HttpRequest request;
    std::istringstream iss(requestStr);
    std::string line;

    // 解析请求行
    if (std::getline(iss, line)) {
        size_t methodEnd = line.find(' ');
        if (methodEnd != std::string::npos) {
            request.method = line.substr(0, methodEnd);

            size_t pathEnd = line.find(' ', methodEnd + 1);
            if (pathEnd != std::string::npos) {
                request.path = line.substr(methodEnd + 1, pathEnd - methodEnd - 1);
            }
        }
    }

    // 解析请求头
    bool contentLengthFound = false;
    int contentLength = 0;
    while (std::getline(iss, line) && line != "\r") {
        // 移除 \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // 转换为小写用于比较
        std::string lowerLine = line;
        for (char& c : lowerLine) {
            c = std::tolower(c);
        }

        // 检查 Content-Type
        if (lowerLine.find("content-type:") == 0) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                request.contentType = line.substr(colonPos + 1);
                // 去除前后空格
                size_t start = request.contentType.find_first_not_of(" \t");
                size_t end = request.contentType.find_last_not_of(" \t");
                if (start != std::string::npos && end != std::string::npos) {
                    request.contentType = request.contentType.substr(start, end - start + 1);
                }
            }
        }

        // 检查 Content-Length
        if (lowerLine.find("content-length:") == 0) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string lengthStr = line.substr(colonPos + 1);
                try {
                    contentLength = std::stoi(lengthStr);
                    contentLengthFound = true;
                } catch (...) {
                    contentLength = 0;
                }
            }
        }
    }

    // 读取请求体
    if (contentLengthFound && contentLength > 0) {
        // 在原始字符串中查找空行后的位置
        size_t headerEnd = requestStr.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            size_t bodyStart = headerEnd + 4;
            if (bodyStart < requestStr.length()) {
                request.body = requestStr.substr(bodyStart, contentLength);
            }
        }
    }

    return request;
}

std::string HttpServer::buildResponse(int statusCode, const std::string& body, const std::string& contentType) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " ";

    switch (statusCode) {
        case 200:
            oss << "OK";
            break;
        case 400:
            oss << "Bad Request";
            break;
        case 404:
            oss << "Not Found";
            break;
        case 500:
            oss << "Internal Server Error";
            break;
        default:
            oss << "Unknown";
            break;
    }

    oss << "\r\n";
    oss << "Content-Type: " << contentType << "; charset=utf-8\r\n";
    oss << "Content-Length: " << body.length() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body;

    return oss.str();
}

bool HttpServer::isValidJson(const std::string& str) {
    if (str.empty()) {
        return false;
    }

    // 去除首尾空白
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return false;
    }
    std::string trimmed = str.substr(start, end - start + 1);

    // 简单 JSON 格式验证
    // 必须以 { 开头，以 } 结尾
    if (trimmed.empty() || trimmed[0] != '{') {
        return false;
    }
    if (trimmed[trimmed.length() - 1] != '}') {
        return false;
    }

    // 至少需要有内容在 {} 之间
    if (trimmed.length() < 3) {
        return false;
    }

    // 检查基本的括号平衡
    int braceCount = 0;
    bool inString = false;
    bool escape = false;
    bool hasContent = false;

    for (size_t i = 0; i < trimmed.length(); ++i) {
        char c = trimmed[i];

        if (escape) {
            escape = false;
            continue;
        }

        if (c == '\\') {
            escape = true;
            continue;
        }

        if (c == '"') {
            inString = !inString;
            if (inString) {
                hasContent = true;
            }
            continue;
        }

        if (!inString) {
            if (c == '{') {
                braceCount++;
            } else if (c == '}') {
                braceCount--;
            } else if (c == ':') {
                hasContent = true;
            }
        }
    }

    return !inString && braceCount == 0 && hasContent;
}

std::string HttpServer::extractSqlFromJson(const std::string& jsonBody) {
    // 首先验证 JSON 格式：必须以 { 开头，以 } 结尾
    if (jsonBody.empty() || jsonBody[0] != '{' || jsonBody[jsonBody.length() - 1] != '}') {
        return "";
    }

    // 简单的 JSON 解析：查找 "sql": "..."
    const std::string key = "\"sql\"";
    size_t keyPos = jsonBody.find(key);
    if (keyPos == std::string::npos) {
        return "";
    }

    size_t colonPos = jsonBody.find(':', keyPos);
    if (colonPos == std::string::npos) {
        return "";
    }

    // 查找第一个引号
    size_t quoteStart = jsonBody.find('\"', colonPos + 1);
    if (quoteStart == std::string::npos) {
        return "";
    }

    // 查找结束引号（考虑转义）
    quoteStart++;  // 跳过开始引号
    size_t quoteEnd = quoteStart;
    while (quoteEnd < jsonBody.length()) {
        if (jsonBody[quoteEnd] == '\"') {
            // 检查是否转义
            if (quoteEnd == 0 || jsonBody[quoteEnd - 1] != '\\') {
                break;
            }
        }
        quoteEnd++;
    }

    if (quoteEnd >= jsonBody.length()) {
        return "";
    }

    return jsonBody.substr(quoteStart, quoteEnd - quoteStart);
}

} // namespace MemoryDB
