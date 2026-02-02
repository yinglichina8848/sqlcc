#ifndef SQLCC_TABLE_STORAGE_H
#define SQLCC_TABLE_STORAGE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

// 前向声明解决循环依赖
namespace sqlcc {
class StorageEngine;
class Page;
class BPlusTreeIndex;
class IndexManager;
}

namespace sqlcc {

// 表数据存储格式定义
const size_t PAGE_HEADER_SIZE = 24;           // 页面头部大小
const size_t SLOT_ARRAY_ENTRY_SIZE = 4;       // 槽数组每个条目大小
const size_t MAX_RECORD_SIZE = 8192;          // 最大记录大小

// 页面类型枚举
enum class PageType : uint8_t {
    INVALID_PAGE = 0,
    TABLE_PAGE,
    INDEX_PAGE,
    SYSTEM_PAGE
};

// 页面头部结构
struct PageHeader {
    PageType page_type;           // 页面类型
    int32_t page_id;              // 页面ID
    int32_t prev_page_id;         // 前一页ID
    int32_t next_page_id;         // 后一页ID
    uint16_t free_space_offset;   // 空闲空间起始位置偏移量
    uint16_t free_space_size;     // 空闲空间大小
    uint16_t slot_count;          // 槽的数量
    uint16_t tuple_count;         // 元组数量
};

// 记录头部结构
struct RecordHeader {
    uint32_t size;                // 记录大小（包括头部）
    bool is_deleted;              // 是否被删除
    uint32_t next_free_offset;    // 下一个空闲记录偏移量（用于记录回收）
};

// 表列信息
struct TableColumn {
    std::string name;             // 列名
    std::string type;             // 数据类型
    size_t size;                  // 数据大小
    bool nullable;                // 是否可为空
    std::string default_value;    // 默认值
};

// 表元数据
struct TableMetadata {
    int64_t table_id;                           // 表ID
    std::string table_name;                     // 表名
    std::string database_name;                  // 数据库名
    std::vector<TableColumn> columns;           // 列信息
    std::unordered_map<std::string, int> column_index_map; // 列名到索引的映射
    size_t record_size;                         // 固定记录大小（对于定长记录）
    bool is_fixed_length;                       // 是否为定长记录
};

/**
 * @class TableStorageManager
 * @brief 表存储管理器 - 实现堆表（Heap Table）的物理组织与记录管理
 *
 * WHY层 - 设计意图：
 *   数据库需要将逻辑上的“行（Rows）”转换为物理上的“字节（Bytes）”。
 *   TableStorageManager 负责管理跨多个页面的记录存储，解决页面内空间分配（Slotted Pages）、
 *   记录跨页（Spanning - 简化版暂不支持）以及删除记录后的空间回收问题。
 *
 * WHAT层 - 功能说明：
 *   实现 CRUD 操作：InsertRecord, UpdateRecord, DeleteRecord, GetRecord。
 *   支持全表扫描（ScanTable），返回所有活跃记录的物理位置（RID: PageID + Offset）。
 *   管理表元数据（TableMetadata）和相关的 B+ 树索引。
 *
 * HOW层 - 实现机制：
 *   1. 槽位管理：采用 Slotted Page 架构，页面头部维护槽数组，每个槽记录对应记录在页内的偏移量。
 *   2. 顺序扫描：ScanTable 通过遍历页面的 NextPageID 链表并解析每页的槽数组实现。
 *   3. 序列化：根据 TableMetadata 定义的 Schema，将字符串数组转换为紧凑的二进制表示。
 *   4. 空间复用：DeleteRecord 仅标记 is_deleted 位，后续插入操作可重用这些空隙。
 */
class TableStorageManager {
public:
    TableStorageManager(std::shared_ptr<StorageEngine> storage_engine);
    ~TableStorageManager();

    // 表管理
    bool CreateTable(const std::string& table_name, const std::vector<TableColumn>& columns);
    bool DropTable(const std::string& table_name);
    bool TableExists(const std::string& table_name) const;
    std::shared_ptr<TableMetadata> GetTableMetadata(const std::string& table_name) const;

    // 记录操作
    bool InsertRecord(const std::string& table_name, const std::vector<std::string>& values, int32_t& page_id, size_t& offset);
    bool UpdateRecord(const std::string& table_name, int32_t page_id, size_t offset, const std::vector<std::string>& new_values);
    bool DeleteRecord(const std::string& table_name, int32_t page_id, size_t offset);
    std::vector<std::string> GetRecord(const std::string& table_name, int32_t page_id, size_t offset) const;
    
    // 批量操作
    std::vector<std::pair<int32_t, size_t>> ScanTable(const std::string& table_name) const;
    std::vector<std::vector<std::string>> GetRecords(const std::string& table_name, 
                                                     const std::vector<std::pair<int32_t, size_t>>& locations) const;

    // 索引管理
    bool CreateIndex(const std::string& table_name, const std::string& column_name);
    bool DropIndex(const std::string& table_name, const std::string& column_name);
    bool IndexExists(const std::string& table_name, const std::string& column_name) const;
    class BPlusTreeIndex* GetIndex(const std::string& table_name, const std::string& column_name);

private:
    std::shared_ptr<StorageEngine> storage_engine_;  // 存储引擎
    std::shared_ptr<IndexManager> index_manager_;    // 索引管理器
    std::unordered_map<std::string, std::shared_ptr<TableMetadata>> table_metadata_; // 表元数据映射
    
    // 内部辅助方法
    class Page* AllocateNewPage(const std::string& table_name);
    bool InitializePage(class Page* page, const std::string& table_name);
    bool InsertRecordToPage(class Page* page, const std::vector<std::string>& values, size_t& offset);
    bool UpdateRecordInPage(class Page* page, size_t offset, const std::vector<std::string>& new_values);
    bool DeleteRecordInPage(class Page* page, size_t offset);
    std::vector<std::string> GetRecordFromPage(class Page* page, size_t offset) const;
    size_t CalculateRecordSize(const std::vector<std::string>& values, const TableMetadata& metadata) const;
    void SerializeRecord(const std::vector<std::string>& values, const TableMetadata& metadata, char* buffer) const;
    std::vector<std::string> DeserializeRecord(const char* buffer, const TableMetadata& metadata) const;
    PageHeader ReadPageHeader(class Page* page) const;
    void WritePageHeader(class Page* page, const PageHeader& header) const;
};

} // namespace sqlcc

#endif // SQLCC_TABLE_STORAGE_H
