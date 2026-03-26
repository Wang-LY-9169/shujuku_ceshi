#include "table.h"
#include "encryption.h"
#include <iostream>
#include <iomanip>

namespace MemoryDB {

Table::Table(const std::string& name)
    : name_(name) {
}

void Table::setSchema(const std::vector<ColumnDef>& columns) {
    columns_ = columns;
}

void Table::setCryptoEngine(std::shared_ptr<CryptoEngine> engine) {
    crypto_ = engine;
}

bool Table::isEncrypted() const {
    return crypto_ && crypto_->isEnabled();
}

std::string Table::getName() const {
    return name_;
}

std::vector<ColumnDef> Table::getSchema() const {
    return columns_;
}

size_t Table::getRowCount() const {
    return rows_.size();
}

bool Table::insertRow(const Row& row) {
    // 检查列数是否匹配
    if (row.size() != columns_.size()) {
        return false;
    }

    // 加密数据
    Row encryptedRow = row;
    if (isEncrypted()) {
        for (auto& value : encryptedRow) {
            value = crypto_->encrypt(value);
        }
    }

    rows_.push_back(encryptedRow);
    return true;
}

std::vector<Row> Table::selectRows() const {
    // 解密数据
    std::vector<Row> decryptedRows;
    for (const auto& row : rows_) {
        Row decryptedRow = row;
        if (isEncrypted()) {
            for (auto& value : decryptedRow) {
                value = crypto_->decrypt(value);
            }
        }
        decryptedRows.push_back(decryptedRow);
    }
    return decryptedRows;
}

bool Table::updateRows(int columnIndex, const std::string& newValue) {
    // 检查列索引是否有效
    if (columnIndex < 0 || columnIndex >= static_cast<int>(columns_.size())) {
        return false;
    }

    // 加密新值
    std::string encryptedValue = newValue;
    if (isEncrypted()) {
        encryptedValue = crypto_->encrypt(newValue);
    }

    // 更新所有行的指定列
    for (auto& row : rows_) {
        row[columnIndex] = encryptedValue;
    }
    return true;
}

bool Table::deleteRows() {
    rows_.clear();
    return true;
}

void Table::print() const {
    // 获取解密后的数据
    auto rows = selectRows();

    // 计算每列宽度
    std::vector<size_t> colWidths;
    for (const auto& col : columns_) {
        colWidths.push_back(col.name.length());
    }
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size() && i < colWidths.size(); ++i) {
            colWidths[i] = std::max(colWidths[i], row[i].length());
        }
    }

    // 计算总宽度
    size_t totalWidth = columns_.size() + 1;
    for (size_t w : colWidths) {
        totalWidth += w + 2;
    }

    // 打印分隔线
    std::cout << std::string(totalWidth, '-') << "\n";

    // 打印表头
    std::cout << "|";
    for (size_t i = 0; i < columns_.size(); ++i) {
        std::cout << " " << std::setw(colWidths[i]) << std::left << columns_[i].name << " |";
    }
    std::cout << "\n";

    std::cout << std::string(totalWidth, '-') << "\n";

    // 打印数据行
    for (const auto& row : rows) {
        std::cout << "|";
        for (size_t i = 0; i < row.size() && i < colWidths.size(); ++i) {
            std::cout << " " << std::setw(colWidths[i]) << std::left << row[i] << " |";
        }
        std::cout << "\n";
    }

    std::cout << std::string(totalWidth, '-') << "\n";
    std::cout << rows.size() << " 行";
    if (isEncrypted()) {
        std::cout << " [加密]";
    }
    std::cout << "\n";
}

} // namespace MemoryDB
