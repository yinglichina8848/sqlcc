#include "database_file_manager.h"
#include "../storage_engine/table_storage.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace sqlcc {

// WAL操作类型定义
enum WALOperation {
    OP_BEGIN_TXN = 1,
    OP_COMMIT_TXN = 2,
    OP_ROLLBACK_TXN = 3,
    OP_WRITE_PAGE = 4,
    OP_ALLOCATE_PAGE = 5,
    OP_FREE_PAGE = 6
};

DatabaseFileManager::DatabaseFileManager(const std::string& db_path, size_t page_size)
    : db_path_(db_path), page_size_(page_size), wal_path_(db_path + ".wal") {
}

DatabaseFileManager::~DatabaseFileManager() {
    if (db_file_.is_open()) {
        db_file_.close();
    }
    if (wal_file_.is_open()) {
        wal_file_.close();
    }
}

bool DatabaseFileManager::Initialize() {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        // 检查数据库文件是否存在
        if (fs::exists(db_path_)) {
            // 尝试加载现有数据库
            if (!LoadDatabaseFile()) {
                // 如果加载失败，尝试删除旧文件并重新创建
                try {
                    fs::remove(db_path_);
                } catch (const std::exception&) {
                    // 忽略删除失败的错误
                }

                // 创建新数据库
                if (!CreateDatabaseFile()) {
                    last_error_ = "Failed to create new database file after load failure";
                    return false;
                }
            }
        } else {
            // 创建新数据库
            if (!CreateDatabaseFile()) {
                last_error_ = "Failed to create new database file";
                return false;
            }
        }

        // 初始化WAL文件
        wal_file_.open(wal_path_, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        if (!wal_file_) {
            wal_file_.open(wal_path_, std::ios::out | std::ios::binary);
            if (!wal_file_) {
                last_error_ = "Failed to open WAL file: " + wal_path_;
                return false;
            }
            wal_file_.close();
            wal_file_.open(wal_path_, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        }

        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        last_error_ = "Initialization failed: " + std::string(e.what());
        return false;
    }
}

bool DatabaseFileManager::CreateDatabaseFile() {
    try {
        // 初始化文件头
        header_ = DatabaseFileHeader();
        header_.page_size = page_size_;
        header_.total_pages = 1;  // 至少有一个页（文件头页）

        // 创建数据库文件
        db_file_.open(db_path_, std::ios::out | std::ios::binary);
        if (!db_file_) {
            last_error_ = "Failed to create database file: " + db_path_;
            return false;
        }

        // 写入文件头
        db_file_.write(reinterpret_cast<const char*>(&header_), sizeof(DatabaseFileHeader));

        // 关闭并重新以读写模式打开
        db_file_.close();
        db_file_.open(db_path_, std::ios::in | std::ios::out | std::ios::binary);

        return db_file_.good();
    } catch (const std::exception& e) {
        last_error_ = "Create database file failed: " + std::string(e.what());
        return false;
    }
}

bool DatabaseFileManager::LoadDatabaseFile() {
    try {
        db_file_.open(db_path_, std::ios::in | std::ios::out | std::ios::binary);
        if (!db_file_) {
            last_error_ = "Failed to open database file: " + db_path_;
            return false;
        }

        // 读取文件头
        db_file_.read(reinterpret_cast<char*>(&header_), sizeof(DatabaseFileHeader));
        if (!db_file_ || !header_.IsValid()) {
            last_error_ = "Invalid database file header";
            return false;
        }

        // 验证页大小
        if (header_.page_size != page_size_) {
            last_error_ = "Page size mismatch: expected " + std::to_string(page_size_) +
                         ", got " + std::to_string(header_.page_size);
            return false;
        }

        // 初始化空闲页映射（简化实现）
        free_pages_.clear();
        for (uint64_t i = header_.free_page_start; i < header_.total_pages; ++i) {
            free_pages_[static_cast<uint32_t>(i)] = true;
        }

        return true;
    } catch (const std::exception& e) {
        last_error_ = "Load database file failed: " + std::string(e.what());
        return false;
    }
}

bool DatabaseFileManager::SaveHeader() {
    try {
        db_file_.seekp(0, std::ios::beg);
        db_file_.write(reinterpret_cast<const char*>(&header_), sizeof(DatabaseFileHeader));
        return db_file_.good();
    } catch (const std::exception& e) {
        last_error_ = "Save header failed: " + std::string(e.what());
        return false;
    }
}

bool DatabaseFileManager::ReadPage(uint32_t page_id, void* buffer, size_t buffer_size) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        last_error_ = "DatabaseFileManager not initialized";
        return false;
    }

    if (!ValidatePage(page_id)) {
        last_error_ = "Invalid page ID: " + std::to_string(page_id);
        return false;
    }

    try {
        // 计算页偏移
        size_t offset = sizeof(DatabaseFileHeader) + (page_id - 1) * page_size_;

        // 读取页头
        PageHeader page_header;
        db_file_.seekg(offset, std::ios::beg);
        db_file_.read(reinterpret_cast<char*>(&page_header), sizeof(PageHeader));

        if (!db_file_ || page_header.page_id != page_id) {
            last_error_ = "Failed to read page header for page " + std::to_string(page_id);
            return false;
        }

        // 读取数据（简化实现，不进行校验和验证）
        size_t data_offset = offset + sizeof(PageHeader);
        size_t data_size = std::min(buffer_size, static_cast<size_t>(page_size_ - sizeof(PageHeader)));
        db_file_.seekg(data_offset, std::ios::beg);
        db_file_.read(static_cast<char*>(buffer), data_size);

        return db_file_.good();
    } catch (const std::exception& e) {
        last_error_ = "Read page failed: " + std::string(e.what());
        return false;
    }
}

bool DatabaseFileManager::WritePage(uint32_t page_id, const void* data, size_t data_size) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        last_error_ = "DatabaseFileManager not initialized";
        return false;
    }

    if (!ValidatePage(page_id)) {
        last_error_ = "Invalid page ID: " + std::to_string(page_id);
        return false;
    }

    if (data_size > page_size_ - sizeof(PageHeader)) {
        last_error_ = "Data size too large for page: " + std::to_string(data_size);
        return false;
    }

    try {
        // 计算页偏移
        size_t offset = sizeof(DatabaseFileHeader) + (page_id - 1) * page_size_;

        // 创建页头
        PageHeader page_header;
        page_header.page_type = PageType::TABLE_PAGE;  // 数据页
        page_header.page_id = page_id;
        // 其他字段保持默认值

        // 写入页头
        db_file_.seekp(offset, std::ios::beg);
        db_file_.write(reinterpret_cast<const char*>(&page_header), sizeof(PageHeader));

        // 写入数据
        size_t data_offset = offset + sizeof(PageHeader);
        db_file_.seekp(data_offset, std::ios::beg);
        db_file_.write(static_cast<const char*>(data), data_size);

        // 填充剩余空间为0
        size_t remaining = page_size_ - sizeof(PageHeader) - data_size;
        if (remaining > 0) {
            std::vector<char> zeros(remaining, 0);
            db_file_.write(zeros.data(), zeros.size());
        }

        // 写入WAL日志
        WriteWAL(OP_WRITE_PAGE, page_id, data, data_size);

        return db_file_.good();
    } catch (const std::exception& e) {
        last_error_ = "Write page failed: " + std::string(e.what());
        return false;
    }
}

bool DatabaseFileManager::AllocatePage(uint32_t& page_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        last_error_ = "DatabaseFileManager not initialized";
        return false;
    }

    try {
        // 查找空闲页
        auto it = std::find_if(free_pages_.begin(), free_pages_.end(),
                              [](const auto& pair) { return pair.second; });

        if (it != free_pages_.end()) {
            // 复用空闲页
            page_id = it->first;
            it->second = false;
        } else {
            // 分配新页
            page_id = static_cast<uint32_t>(header_.total_pages + 1);
            header_.total_pages++;

            // 保存更新的头信息
            SaveHeader();
        }

        // 写入WAL日志
        WriteWAL(OP_ALLOCATE_PAGE, page_id, nullptr, 0);

        return true;
    } catch (const std::exception& e) {
        last_error_ = "Allocate page failed: " + std::string(e.what());
        return false;
    }
}

bool DatabaseFileManager::FreePage(uint32_t page_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        last_error_ = "DatabaseFileManager not initialized";
        return false;
    }

    if (!ValidatePage(page_id)) {
        last_error_ = "Invalid page ID: " + std::to_string(page_id);
        return false;
    }

    try {
        // 标记为空闲页
        free_pages_[page_id] = true;

        // 写入WAL日志
        WriteWAL(OP_FREE_PAGE, page_id, nullptr, 0);

        return true;
    } catch (const std::exception& e) {
        last_error_ = "Free page failed: " + std::string(e.what());
        return false;
    }
}

uint64_t DatabaseFileManager::WriteWAL(uint32_t operation, uint32_t page_id, const void* data, size_t data_size) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        uint64_t lsn = next_lsn_++;

        // 创建WAL条目
        size_t entry_size = sizeof(WALEntry) + data_size;
        std::vector<char> entry_buffer(entry_size);

        WALEntry* entry = reinterpret_cast<WALEntry*>(entry_buffer.data());
        entry->lsn = lsn;
        entry->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        entry->operation = operation;
        entry->page_id = page_id;
        entry->data_size = static_cast<uint32_t>(data_size);

        if (data && data_size > 0) {
            memcpy(entry->data, data, data_size);
        }

        // 写入WAL文件
        wal_file_.seekp(0, std::ios::end);
        wal_file_.write(entry_buffer.data(), entry_buffer.size());
        wal_file_.flush();

        return lsn;
    } catch (const std::exception& e) {
        last_error_ = "Write WAL failed: " + std::string(e.what());
        return 0;
    }
}

bool DatabaseFileManager::ReplayWAL(uint64_t from_lsn) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        last_error_ = "DatabaseFileManager not initialized";
        return false;
    }

    try {
        wal_file_.seekg(0, std::ios::beg);

        while (wal_file_) {
            WALEntry entry;
            wal_file_.read(reinterpret_cast<char*>(&entry), sizeof(WALEntry));

            if (!wal_file_) break;

            if (entry.lsn <= from_lsn) {
                // 跳过已处理的条目
                if (entry.data_size > 0) {
                    wal_file_.seekg(entry.data_size, std::ios::cur);
                }
                continue;
            }

            // 重放操作
            switch (entry.operation) {
                case OP_BEGIN_TXN:
                    // 开始事务
                    break;
                case OP_COMMIT_TXN:
                    // 提交事务
                    break;
                case OP_WRITE_PAGE: {
                    // 写入页
                    std::vector<char> data(entry.data_size);
                    if (entry.data_size > 0) {
                        wal_file_.read(data.data(), entry.data_size);
                    }
                    WritePage(entry.page_id, data.data(), entry.data_size);
                    break;
                }
                case OP_ALLOCATE_PAGE:
                    // 分配页
                    break;
                case OP_FREE_PAGE:
                    // 释放页
                    FreePage(entry.page_id);
                    break;
            }
        }

        return true;
    } catch (const std::exception& e) {
        last_error_ = "Replay WAL failed: " + std::string(e.what());
        return false;
    }
}

bool DatabaseFileManager::BeginTransaction(uint64_t& txn_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        last_error_ = "DatabaseFileManager not initialized";
        return false;
    }

    txn_id = next_txn_id_++;
    active_transactions_.push_back(txn_id);

    // 写入WAL日志
    WriteWAL(OP_BEGIN_TXN, 0, &txn_id, sizeof(txn_id));

    return true;
}

bool DatabaseFileManager::CommitTransaction(uint64_t txn_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        last_error_ = "DatabaseFileManager not initialized";
        return false;
    }

    // 从活跃事务列表中移除
    auto it = std::find(active_transactions_.begin(), active_transactions_.end(), txn_id);
    if (it != active_transactions_.end()) {
        active_transactions_.erase(it);
    }

    // 写入WAL日志
    WriteWAL(OP_COMMIT_TXN, 0, &txn_id, sizeof(txn_id));

    return true;
}

bool DatabaseFileManager::RollbackTransaction(uint64_t txn_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        last_error_ = "DatabaseFileManager not initialized";
        return false;
    }

    // 从活跃事务列表中移除
    auto it = std::find(active_transactions_.begin(), active_transactions_.end(), txn_id);
    if (it != active_transactions_.end()) {
        active_transactions_.erase(it);
    }

    // 写入WAL日志
    WriteWAL(OP_ROLLBACK_TXN, 0, &txn_id, sizeof(txn_id));

    return true;
}

uint64_t DatabaseFileManager::GetFreePages() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::count_if(free_pages_.begin(), free_pages_.end(),
                        [](const auto& pair) { return pair.second; });
}

bool DatabaseFileManager::ValidatePage(uint32_t page_id) const {
    return page_id > 0 && page_id <= header_.total_pages;
}

uint32_t DatabaseFileManager::CalculateChecksum(const void* data, size_t size) const {
    if (!data || size == 0) return 0;

    const uint32_t* ptr = static_cast<const uint32_t*>(data);
    uint32_t checksum = 0;

    size_t words = size / sizeof(uint32_t);
    for (size_t i = 0; i < words; ++i) {
        checksum ^= ptr[i];
    }

    // 处理剩余字节
    size_t remaining = size % sizeof(uint32_t);
    if (remaining > 0) {
        uint32_t last_word = 0;
        memcpy(&last_word, static_cast<const char*>(data) + words * sizeof(uint32_t), remaining);
        checksum ^= last_word;
    }

    return checksum;
}

std::string DatabaseFileManager::GetPageFilePath(uint32_t page_id) const {
    return db_path_ + ".page_" + std::to_string(page_id);
}

std::string DatabaseFileManager::GetWALFilePath() const {
    return wal_path_;
}

} // namespace sqlcc
