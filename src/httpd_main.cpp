#include "http_server.h"
#include <iostream>
#include <csignal>

using namespace MemoryDB;

volatile sig_atomic_t stopFlag = 0;

#ifdef _WIN32
#include <windows.h>

BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        stopFlag = 1;
        return TRUE;
    }
    return FALSE;
}

#else
#include <unistd.h>

void signalHandler(int signal) {
    stopFlag = 1;
}

#endif

int main(int argc, char* argv[]) {
    int port = 8080;

    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" || arg == "-p") {
            if (i + 1 < argc) {
                try {
                    port = std::stoi(argv[++i]);
                } catch (...) {
                    std::cerr << "Invalid port number" << std::endl;
                    return 1;
                }
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "MemoryDB HTTP Server\n\n";
            std::cout << "Usage: httpd [options]\n\n";
            std::cout << "Options:\n";
            std::cout << "  --port, -p <port>  Set server port (default: 8080)\n";
            std::cout << "  --help, -h         Show this help\n";
            return 0;
        }
    }

#ifdef _WIN32
    SetConsoleCtrlHandler(consoleHandler, TRUE);
#else
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
#endif

    HttpServer server(port);
    server.start();

    return 0;
}
