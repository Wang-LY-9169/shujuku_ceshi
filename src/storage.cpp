#include "storage.h"
#include <iostream>

namespace MemoryDB {

StorageEngine::StorageEngine() {
}

StorageEngine::~StorageEngine() {
}

bool StorageEngine::createTable(const std::string& name, const std::vector<ColumnDef>& schema) {
    // 检查表是否已存在
    if (tables_.find(name) != tables_.end()) {
        return false;
    }

    // 创建新表
    auto table = std::make_unique<Table>(name);
    table->setSchema(schema);
    tables_[name] = std::move(table);

    return true;
}

bool StorageEngine::dropTable(const std::string& name) {
    auto it = tables_.find(name);
    if (it == tables_.end()) {
        return false;
    }
    tables_.erase(it);
    return true;
}

Table* StorageEngine::getTable(const std::string& name) {
    auto it = tables_.find(name);
    if (it != tables_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::string> StorageEngine::listTables() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : tables_) {
        names.push_back(name);
    }
    return names;
}

bool StorageEngine::tableExists(const std::string& name) const {
    return tables_.find(name) != tables_.end();
}

} // namespace MemoryDB
