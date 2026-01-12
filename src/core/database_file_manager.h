#ifndef SQLCC_DATABASE_FILE_MANAGER_H
#define SQLCC_DATABASE_FILE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <cstring>
#include <chrono>

namespace sqlcc {

// 数据库文件头结构
#pragma pack(push, 1)
struct DatabaseFileHeader {
    char magic[8] = {'S', 'Q', 'L', 'C', 'C', 'D', 'B', '\0'};  // 魔数
    uint32_t version = 1;                                       // 版本号
    uint64_t page_size = 4096;                                 // 页大小
    uint64_t total_pages = 0;                                  // 总页数
    uint64_t free_page_start = 1;                              // 空闲页起始位置
    uint64_t root_page_id = 0;                                 // 根页ID
    uint64_t wal_start_lsn = 0;                                // WAL起始LSN
    char reserved[64] = {0};                                   // 保留字段

    // 检查魔数是否正确
    bool IsValid() const {
        return memcmp(magic, "SQLCCDB", 8) == 0;
    }
};
#pragma pack(pop)

// 页头结构


// WAL日志条目结构
#pragma pack(push, 1)
struct WALEntry {
    uint64_t lsn = 0;              // 日志序列号
    uint64_t timestamp = 0;        // 时间戳
    uint32_t operation = 0;        // 操作类型
    uint32_t page_id = 0;          // 页ID
    uint32_t data_size = 0;        // 数据大小
    char data[0];                  // 实际数据（变长）
};
#pragma pack(pop)

// 数据库文件管理器类
class DatabaseFileManager {
public:
    // 构造函数和析构函数
    DatabaseFileManager(const std::string& db_path, size_t page_size = 4096);
    ~DatabaseFileManager();

    // 初始化数据库文件
    bool Initialize();

    // 文件操作
    bool ReadPage(uint32_t page_id, void* buffer, size_t buffer_size);
    bool WritePage(uint32_t page_id, const void* data, size_t data_size);
    bool AllocatePage(uint32_t& page_id);
    bool FreePage(uint32_t page_id);

    // WAL操作
    uint64_t WriteWAL(uint32_t operation, uint32_t page_id, const void* data, size_t data_size);
    bool ReplayWAL(uint64_t from_lsn = 0);

    // 事务支持
    bool BeginTransaction(uint64_t& txn_id);
    bool CommitTransaction(uint64_t txn_id);
    bool RollbackTransaction(uint64_t txn_id);

    // 状态查询
    bool IsInitialized() const { return initialized_; }
    uint64_t GetTotalPages() const { return header_.total_pages; }
    uint64_t GetFreePages() const;

    // 错误处理
    const std::string& GetLastError() const { return last_error_; }

private:
    // 内部辅助方法
    bool CreateDatabaseFile();
    bool LoadDatabaseFile();
    bool SaveHeader();
    bool ValidatePage(uint32_t page_id) const;
    uint32_t CalculateChecksum(const void* data, size_t size) const;
    std::string GetPageFilePath(uint32_t page_id) const;
    std::string GetWALFilePath() const;

    // 成员变量
    std::string db_path_;                    // 数据库路径
    std::string wal_path_;                   // WAL文件路径
    size_t page_size_;                       // 页大小
    bool initialized_ = false;               // 是否已初始化

    DatabaseFileHeader header_;              // 文件头
    std::fstream db_file_;                   // 数据库文件流
    std::fstream wal_file_;                  // WAL文件流

    std::unordered_map<uint32_t, bool> free_pages_;  // 空闲页映射
    std::vector<uint64_t> active_transactions_;      // 活跃事务列表

    mutable std::mutex mutex_;               // 线程安全
    std::string last_error_;                 // 最后错误信息

    // WAL相关
    uint64_t next_lsn_ = 1;                  // 下一个LSN
    uint64_t next_txn_id_ = 1;               // 下一个事务ID
};

} // namespace sqlcc

#endif // SQLCC_DATABASE_FILE_MANAGER_H