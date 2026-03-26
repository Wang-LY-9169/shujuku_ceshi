#include "api_serializer.h"
#include <sstream>

namespace MemoryDB {

std::string ApiSerializer::escapeString(const std::string& str) {
    std::ostringstream escaped;
    escaped << '"';

    for (char c : str) {
        switch (c) {
            case '"':
                escaped << "\\\"";
                break;
            case '\\':
                escaped << "\\\\";
                break;
            case '\b':
                escaped << "\\b";
                break;
            case '\f':
                escaped << "\\f";
                break;
            case '\n':
                escaped << "\\n";
                break;
            case '\r':
                escaped << "\\r";
                break;
            case '\t':
                escaped << "\\t";
                break;
            default:
                if (c >= 0 && c < 0x20) {
                    // 控制字符转义为 \uXXXX
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04X", static_cast<unsigned char>(c));
                    escaped << buf;
                } else {
                    escaped << c;
                }
                break;
        }
    }

    escaped << '"';
    return escaped.str();
}

std::string ApiSerializer::serializeArray(const std::vector<std::string>& arr) {
    if (arr.empty()) {
        return "[]";
    }

    std::ostringstream oss;
    oss << '[';

    for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0) {
            oss << ',';
        }
        oss << escapeString(arr[i]);
    }

    oss << ']';
    return oss.str();
}

std::string ApiSerializer::serializeRows(const std::vector<std::vector<std::string>>& rows) {
    if (rows.empty()) {
        return "[]";
    }

    std::ostringstream oss;
    oss << '[';

    for (size_t i = 0; i < rows.size(); ++i) {
        if (i > 0) {
            oss << ',';
        }
        oss << serializeArray(rows[i]);
    }

    oss << ']';
    return oss.str();
}

std::string ApiSerializer::serialize(const QueryResult& result) {
    std::ostringstream oss;

    oss << "{";
    oss << "\"success\":" << (result.success ? "true" : "false") << ",";
    oss << "\"message\":" << escapeString(result.message) << ",";
    oss << "\"affectedRows\":" << result.affectedRows << ",";
    oss << "\"columns\":" << serializeArray(result.columns) << ",";
    oss << "\"rows\":" << serializeRows(result.rows);
    oss << "}";

    return oss.str();
}

std::string ApiSerializer::error(const std::string& message, int errorCode) {
    std::ostringstream oss;

    oss << "{";
    oss << "\"success\":false,";
    oss << "\"errorCode\":" << errorCode << ",";
    oss << "\"message\":" << escapeString(message);
    oss << "}";

    return oss.str();
}

} // namespace MemoryDB
