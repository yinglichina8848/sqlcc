#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <chrono>
#include <mutex>

namespace sqlcc {
namespace storage {

// 前向声明
class StorageEngine;
class TableStorage;
class IndexManager;

/**
 * @brief 分区定义基类
 * 
 * 定义分区表的基本属性和行为
 */
class Partition {
public:
    enum class Type {
        RANGE,      // 范围分区
        HASH,       // 哈希分区
        LIST,       // 列表分区
        COMPOSITE   // 复合分区
    };

    enum class State {
        ACTIVE,     // 活跃分区
        INACTIVE,   // 非活跃分区
        DROPPING,   // 删除中
        MERGING     // 合并中
    };

    Partition(int32_t partition_id, const std::string& name, Type type);
    virtual ~Partition() = default;

    // 基本属性
    int32_t getPartitionId() const { return partition_id_; }
    const std::string& getName() const { return name_; }
    Type getType() const { return type_; }
    State getState() const { return state_; }
    
    // 分区管理
    virtual bool containsValue(const void* value) const = 0;
    virtual int32_t getPartitionForValue(const void* value) const = 0;
    virtual bool isEmpty() const = 0;
    virtual size_t getRowCount() const = 0;
    virtual uint64_t getSizeInBytes() const = 0;
    
    // 分区状态管理
    void setState(State state) { state_ = state; }
    void setLastModified(const std::chrono::system_clock::time_point& time) { 
        last_modified_ = time; 
    }
    const std::chrono::system_clock::time_point& getLastModified() const { 
        return last_modified_; 
    }

    // 统计信息
    struct PartitionStatistics {
        size_t row_count = 0;
        uint64_t data_size = 0;
        uint64_t index_size = 0;
        std::chrono::system_clock::time_point last_analyzed;
        std::chrono::system_clock::time_point last_modified;
    };

    virtual PartitionStatistics getStatistics() const = 0;
    virtual void updateStatistics() = 0;

protected:
    int32_t partition_id_;
    std::string name_;
    Type type_;
    State state_;
    std::chrono::system_clock::time_point last_modified_;
};

/**
 * @brief 范围分区实现
 */
class RangePartition : public Partition {
public:
    struct Range {
        std::string min_value;
        std::string max_value;
        bool is_min_inclusive;
        bool is_max_inclusive;
    };

    RangePartition(int32_t partition_id, 
                   const std::string& name,
                   const Range& range);

    bool containsValue(const void* value) const override;
    int32_t getPartitionForValue(const void* value) const override;
    bool isEmpty() const override;
    size_t getRowCount() const override;
    uint64_t getSizeInBytes() const override;
    PartitionStatistics getStatistics() const override;
    void updateStatistics() override;

    const Range& getRange() const { return range_; }
    void setRange(const Range& range) { range_ = range; }

private:
    Range range_;
    PartitionStatistics stats_;
};

/**
 * @brief 哈希分区实现
 */
class HashPartition : public Partition {
public:
    struct HashInfo {
        int32_t partition_count;
        std::hash<std::string> hasher;
    };

    HashPartition(int32_t partition_id,
                  const std::string& name,
                  const HashInfo& hash_info);

    bool containsValue(const void* value) const override;
    int32_t getPartitionForValue(const void* value) const override;
    bool isEmpty() const override;
    size_t getRowCount() const override;
    uint64_t getSizeInBytes() const override;
    PartitionStatistics getStatistics() const override;
    void updateStatistics() override;

    const HashInfo& getHashInfo() const { return hash_info_; }

private:
    HashInfo hash_info_;
    PartitionStatistics stats_;
    int32_t hashValue(const void* value) const;
};

/**
 * @brief 列表分区实现
 */
class ListPartition : public Partition {
public:
    struct ValueList {
        std::vector<std::string> values;
        bool is_default;
    };

    ListPartition(int32_t partition_id,
                  const std::string& name,
                  const ValueList& value_list);

    bool containsValue(const void* value) const override;
    int32_t getPartitionForValue(const void* value) const override;
    bool isEmpty() const override;
    size_t getRowCount() const override;
    uint64_t getSizeInBytes() const override;
    PartitionStatistics getStatistics() const override;
    void updateStatistics() override;

    const ValueList& getValueList() const { return value_list_; }
    void setValueList(const ValueList& value_list) { value_list_ = value_list; }

private:
    ValueList value_list_;
    PartitionStatistics stats_;
};

/**
 * @brief 复合分区实现
 */
class CompositePartition : public Partition {
public:
    struct CompositeInfo {
        Type primary_type;    // 主分区类型
        Type secondary_type;  // 子分区类型
        int32_t sub_partition_count;
    };

    CompositePartition(int32_t partition_id,
                       const std::string& name,
                       const CompositeInfo& composite_info);

    bool containsValue(const void* value) const override;
    int32_t getPartitionForValue(const void* value) const override;
    bool isEmpty() const override;
    size_t getRowCount() const override;
    uint64_t getSizeInBytes() const override;
    PartitionStatistics getStatistics() const override;
    void updateStatistics() override;

    const CompositeInfo& getCompositeInfo() const { return composite_info_; }

private:
    CompositeInfo composite_info_;
    PartitionStatistics stats_;
};

/**
 * @brief 分区表定义
 */
struct PartitionedTableDef {
    std::string table_name;
    std::string database_name;
    Partition::Type partition_type;
    std::string partition_column;
    std::vector<std::unique_ptr<Partition>> partitions;
    bool is_sub_partitioned;
    Partition::Type sub_partition_type;
    std::string sub_partition_column;
    
    // 分区选项
    struct PartitionOptions {
        bool auto_partition = false;
        int32_t max_partitions = 1024;
        int32_t threshold_rows = 1000000;
        bool enable_statistics = true;
        bool enable_compression = false;
    } options;
};

/**
 * @brief 分区元数据
 */
struct PartitionMetadata {
    int32_t partition_id;
    std::string table_name;
    std::string database_name;
    std::string partition_name;
    Partition::Type type;
    std::string partition_column;
    std::string storage_path;
    std::chrono::system_clock::time_point created_time;
    std::chrono::system_clock::time_point last_modified;
    Partition::State state;
    size_t row_count;
    uint64_t size_in_bytes;
    
    // 分区特定的元数据
    std::string partition_definition;  // JSON格式的分区定义
    std::string statistics_json;       // 统计信息
};

/**
 * @brief 分区表管理器
 * 
 * 负责分区表的创建、管理和维护
 */
class PartitionManager {
public:
    PartitionManager(std::shared_ptr<StorageEngine> storage_engine,
                    std::shared_ptr<TableStorage> table_storage,
                    std::shared_ptr<IndexManager> index_manager);
    ~PartitionManager() = default;

    // 分区表管理
    bool createPartitionedTable(const PartitionedTableDef& definition);
    bool dropPartitionedTable(const std::string& table_name);
    bool isPartitionedTable(const std::string& table_name) const;
    
    // 分区管理
    bool addPartition(const std::string& table_name, std::unique_ptr<Partition> partition);
    bool dropPartition(const std::string& table_name, int32_t partition_id);
    
    // 分区查询优化
    std::vector<int32_t> getEligiblePartitions(const std::string& table_name,
                                              const void* condition) const;
    
    // 分区信息查询
    std::shared_ptr<Partition> getPartition(const std::string& table_name, 
                                           int32_t partition_id) const;
    std::vector<std::shared_ptr<Partition>> getAllPartitions(const std::string& table_name) const;
    std::vector<std::shared_ptr<PartitionMetadata>> getPartitionMetadata(const std::string& table_name) const;
    
    // 分区统计和维护
    bool updatePartitionStatistics(const std::string& table_name, int32_t partition_id);
    
    // 分区管理操作
    bool truncatePartition(const std::string& table_name, int32_t partition_id);
    
    // 监听器
    using PartitionChangeListener = std::function<void(const std::string& table_name, 
                                                      int32_t partition_id, 
                                                      const std::string& operation)>;
    void addPartitionChangeListener(const std::string& table_name, 
                                   PartitionChangeListener listener);

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<TableStorage> table_storage_;
    std::shared_ptr<IndexManager> index_manager_;
    
    // 分区表存储
    std::unordered_map<std::string, std::unique_ptr<PartitionedTableDef>> partitioned_tables_;
    std::unordered_map<std::string, std::unordered_map<int32_t, std::shared_ptr<Partition>>> partitions_;
    
    // 分区元数据存储
    std::unordered_map<std::string, std::unordered_map<int32_t, std::unique_ptr<PartitionMetadata>>> partition_metadata_;
    
    // 监听器管理
    std::unordered_map<std::string, std::vector<PartitionChangeListener>> change_listeners_;
    mutable std::mutex mutex_;
    
    // 内部方法
    std::string getPartitionStoragePath(const std::string& table_name, 
                                       int32_t partition_id) const;
    void notifyPartitionChange(const std::string& table_name,
                              int32_t partition_id,
                              const std::string& operation);
    
    // 分区选择算法
    std::vector<int32_t> selectPartitionsByCondition(const std::string& table_name,
                                                    const void* condition) const;
    int32_t calculatePartitionHash(const void* value) const;
    
    // 统计信息管理
    bool collectPartitionStatistics(const std::string& table_name,
                                   int32_t partition_id,
                                   Partition::PartitionStatistics& stats) const;
};

} // namespace storage
} // namespace sqlcc
