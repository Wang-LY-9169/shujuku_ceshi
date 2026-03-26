cmake_minimum_required(VERSION 3.15)
project(MemoryDB VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 公共源文件
set(COMMON_SOURCES
    src/database.cpp
    src/sql_parser.cpp
    src/table.cpp
    src/encryption.cpp
    src/storage.cpp
    src/api_serializer.cpp
)

# 公共头文件
set(HEADERS
    include/database.h
    include/sql_parser.h
    include/table.h
    include/encryption.h
    include/storage.h
    include/api_serializer.h
    include/http_server.h
)

# CLI 可执行文件
add_executable(memorydb src/main.cpp ${COMMON_SOURCES} ${HEADERS})
target_include_directories(memorydb PRIVATE include)

# HTTP 服务可执行文件
add_executable(httpd src/httpd_main.cpp src/http_server.cpp ${COMMON_SOURCES} ${HEADERS})
target_include_directories(httpd PRIVATE include)

if(WIN32)
    target_link_libraries(httpd ws2_32)
endif()

# API 测试可执行文件
add_executable(test_api tests/test_api.cpp src/http_server.cpp ${COMMON_SOURCES} ${HEADERS})
target_include_directories(test_api PRIVATE include)

if(WIN32)
    target_link_libraries(test_api ws2_32)
endif()

# 测试
enable_testing()
add_subdirectory(tests)
