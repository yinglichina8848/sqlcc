#include "storage/partition_manager.h"
#include "storage/table_storage.h"
#include "storage/b_plus_tree.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include "utils/config_snapshot.h"

namespace sqlcc {
namespace storage {

// ========== Partition基类实现 ==========

Partition::Partition(int32_t partition_id, const std::string& name, Type type)
    : partition_id_(partition_id), name_(name), type_(type), state_(State::ACTIVE) {
    last_modified_ = std::chrono::system_clock::now();
}

// ========== RangePartition实现 ==========

RangePartition::RangePartition(int32_t partition_id, 
                               const std::string& name,
                               const Range& range)
    : Partition(partition_id, name, Type::RANGE), range_(range) {
}

bool RangePartition::containsValue(const void* value) const {
    // 简化实现 - 假设value是字符串指针
    const std::string* str_value = static_cast<const std::string*>(value);
    if (!str_value) return false;
    
    // 比较字符串值在范围内
    // 简化实现
    return !range_.min_value.empty() && !range_.max_value.empty();
}

int32_t RangePartition::getPartitionForValue(const void* value) const {
    return containsValue(value) ? partition_id_ : -1;
}

bool RangePartition::isEmpty() const {
    return stats_.row_count == 0;
}

size_t RangePartition::getRowCount() const {
    return stats_.row_count;
}

uint64_t RangePartition::getSizeInBytes() const {
    return stats_.data_size + stats_.index_size;
}

Partition::PartitionStatistics RangePartition::getStatistics() const {
    return stats_;
}

void RangePartition::updateStatistics() {
    stats_.last_analyzed = std::chrono::system_clock::now();
    stats_.last_modified = last_modified_;
}

// ========== HashPartition实现 ==========

HashPartition::HashPartition(int32_t partition_id,
                             const std::string& name,
                             const HashInfo& hash_info)
    : Partition(partition_id, name, Type::HASH), hash_info_(hash_info) {
}

int32_t HashPartition::hashValue(const void* value) const {
    // 简化实现 - 假设value是字符串指针
    const std::string* str_value = static_cast<const std::string*>(value);
    std::string key = str_value ? *str_value : "";
    return hash_info_.hasher(key) % hash_info_.partition_count;
}

bool HashPartition::containsValue(const void* value) const {
    return hashValue(value) == partition_id_;
}

int32_t HashPartition::getPartitionForValue(const void* value) const {
    return hashValue(value);
}

bool HashPartition::isEmpty() const {
    return stats_.row_count == 0;
}

size_t HashPartition::getRowCount() const {
    return stats_.row_count;
}

uint64_t HashPartition::getSizeInBytes() const {
    return stats_.data_size + stats_.index_size;
}

Partition::PartitionStatistics HashPartition::getStatistics() const {
    return stats_;
}

void HashPartition::updateStatistics() {
    stats_.last_analyzed = std::chrono::system_clock::now();
    stats_.last_modified = last_modified_;
}

// ========== ListPartition实现 ==========

ListPartition::ListPartition(int32_t partition_id,
                             const std::string& name,
                             const ValueList& value_list)
    : Partition(partition_id, name, Type::LIST), value_list_(value_list) {
}

bool ListPartition::containsValue(const void* value) const {
    const std::string* str_value = static_cast<const std::string*>(value);
    if (!str_value) return false;
    
    for (const auto& list_value : value_list_.values) {
        if (*str_value == list_value) {
            return true;
        }
    }
    return false;
}

int32_t ListPartition::getPartitionForValue(const void* value) const {
    return containsValue(value) ? partition_id_ : -1;
}

bool ListPartition::isEmpty() const {
    return stats_.row_count == 0;
}

size_t ListPartition::getRowCount() const {
    return stats_.row_count;
}

uint64_t ListPartition::getSizeInBytes() const {
    return stats_.data_size + stats_.index_size;
}

Partition::PartitionStatistics ListPartition::getStatistics() const {
    return stats_;
}

void ListPartition::updateStatistics() {
    stats_.last_analyzed = std::chrono::system_clock::now();
    stats_.last_modified = last_modified_;
}

// ========== CompositePartition实现 ==========

CompositePartition::CompositePartition(int32_t partition_id,
                                       const std::string& name,
                                       const CompositeInfo& composite_info)
    : Partition(partition_id, name, Type::COMPOSITE), composite_info_(composite_info) {
}

bool CompositePartition::containsValue(const void* /*value*/) const {
    // 简化实现
    return true;
}

int32_t CompositePartition::getPartitionForValue(const void* /*value*/) const {
    // 计算主分区
    return partition_id_;
}

bool CompositePartition::isEmpty() const {
    return stats_.row_count == 0;
}

size_t CompositePartition::getRowCount() const {
    return stats_.row_count;
}

uint64_t CompositePartition::getSizeInBytes() const {
    return stats_.data_size + stats_.index_size;
}

Partition::PartitionStatistics CompositePartition::getStatistics() const {
    return stats_;
}

void CompositePartition::updateStatistics() {
    stats_.last_analyzed = std::chrono::system_clock::now();
    stats_.last_modified = last_modified_;
}

// ========== PartitionManager实现 ==========

PartitionManager::PartitionManager(std::shared_ptr<StorageEngine> storage_engine,
                                  std::shared_ptr<TableStorage> table_storage,
                                  std::shared_ptr<IndexManager> index_manager)
    : storage_engine_(std::move(storage_engine)),
      table_storage_(std::move(table_storage)),
      index_manager_(std::move(index_manager)) {
}

bool PartitionManager::createPartitionedTable(const PartitionedTableDef& definition) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查表是否已存在
    if (partitioned_tables_.find(definition.table_name) != partitioned_tables_.end()) {
        return false;
    }
    
    // 创建分区表定义副本
    auto table_def = std::make_unique<PartitionedTableDef>();
    table_def->table_name = definition.table_name;
    table_def->database_name = definition.database_name;
    table_def->partition_type = definition.partition_type;
    table_def->partition_column = definition.partition_column;
    table_def->is_sub_partitioned = definition.is_sub_partitioned;
    table_def->sub_partition_type = definition.sub_partition_type;
    table_def->sub_partition_column = definition.sub_partition_column;
    table_def->options = definition.options;
    
    // 为每个分区创建存储路径
    for (size_t i = 0; i < table_def->partitions.size(); ++i) {
        auto& partition = table_def->partitions[i];
        partition->setLastModified(std::chrono::system_clock::now());
        
        // 创建分区存储目录
        std::string partition_path = getPartitionStoragePath(definition.table_name, partition->getPartitionId());
        std::filesystem::create_directories(partition_path);
        
        // 创建分区元数据
        auto metadata = std::make_unique<PartitionMetadata>();
        metadata->partition_id = partition->getPartitionId();
        metadata->table_name = definition.table_name;
        metadata->database_name = definition.database_name;
        metadata->partition_name = partition->getName();
        metadata->type = partition->getType();
        metadata->partition_column = definition.partition_column;
        metadata->storage_path = partition_path;
        metadata->created_time = std::chrono::system_clock::now();
        metadata->last_modified = std::chrono::system_clock::now();
        metadata->state = Partition::State::ACTIVE;
        metadata->row_count = 0;
        metadata->size_in_bytes = 0;
        
        // 保存元数据
        partition_metadata_[definition.table_name][partition->getPartitionId()] = std::move(metadata);
    }
    
    // 保存表定义
    partitioned_tables_[definition.table_name] = std::move(table_def);
    
    // 初始化分区映射
    for (const auto& partition : partitioned_tables_[definition.table_name]->partitions) {
        partitions_[definition.table_name][partition->getPartitionId()] = std::shared_ptr<Partition>(partition.get());
    }
    
    return true;
}

bool PartitionManager::dropPartitionedTable(const std::string& table_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = partitioned_tables_.find(table_name);
    if (it == partitioned_tables_.end()) {
        return false;
    }
    
    // 删除所有分区的存储文件
    for (const auto& [partition_id, partition] : partitions_[table_name]) {
        std::string partition_path = getPartitionStoragePath(table_name, partition_id);
        try {
            std::filesystem::remove_all(partition_path);
        } catch (const std::exception& e) {
            // 日志记录错误但继续删除
        }
    }
    
    // 清理内存中的数据
    partitioned_tables_.erase(table_name);
    partitions_.erase(table_name);
    partition_metadata_.erase(table_name);
    change_listeners_.erase(table_name);
    
    return true;
}

bool PartitionManager::isPartitionedTable(const std::string& table_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return partitioned_tables_.find(table_name) != partitioned_tables_.end();
}

bool PartitionManager::addPartition(const std::string& table_name, std::unique_ptr<Partition> partition) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto table_it = partitioned_tables_.find(table_name);
    if (table_it == partitioned_tables_.end()) {
        return false;
    }
    
    // 检查分区ID是否已存在
    auto partition_it = partitions_[table_name].find(partition->getPartitionId());
    if (partition_it != partitions_[table_name].end()) {
        return false;
    }
    
    // 设置分区状态和时间
    partition->setLastModified(std::chrono::system_clock::now());
    
    // 创建分区存储目录
    std::string partition_path = getPartitionStoragePath(table_name, partition->getPartitionId());
    std::filesystem::create_directories(partition_path);
    
    // 创建分区元数据
    auto metadata = std::make_unique<PartitionMetadata>();
    metadata->partition_id = partition->getPartitionId();
    metadata->table_name = table_name;
    metadata->database_name = table_it->second->database_name;
    metadata->partition_name = partition->getName();
    metadata->type = partition->getType();
    metadata->partition_column = table_it->second->partition_column;
    metadata->storage_path = partition_path;
    metadata->created_time = std::chrono::system_clock::now();
    metadata->last_modified = std::chrono::system_clock::now();
    metadata->state = Partition::State::ACTIVE;
    metadata->row_count = 0;
    metadata->size_in_bytes = 0;
    
    // 保存分区和元数据
    partitions_[table_name][partition->getPartitionId()] = std::shared_ptr<Partition>(partition.release());
    partition_metadata_[table_name][partition->getPartitionId()] = std::move(metadata);
    
    // 通知监听器
    notifyPartitionChange(table_name, partition->getPartitionId(), "ADD_PARTITION");
    
    return true;
}

bool PartitionManager::dropPartition(const std::string& table_name, int32_t partition_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto table_it = partitions_.find(table_name);
    if (table_it == partitions_.end()) {
        return false;
    }
    
    auto partition_it = table_it->second.find(partition_id);
    if (partition_it == table_it->second.end()) {
        return false;
    }
    
    // 设置分区状态为删除中
    partition_it->second->setState(Partition::State::DROPPING);
    
    // 删除分区存储目录
    std::string partition_path = getPartitionStoragePath(table_name, partition_id);
    try {
        std::filesystem::remove_all(partition_path);
    } catch (const std::exception& e) {
        // 日志记录错误
    }
    
    // 从内存中移除
    table_it->second.erase(partition_id);
    partition_metadata_[table_name].erase(partition_id);
    
    // 通知监听器
    notifyPartitionChange(table_name, partition_id, "DROP_PARTITION");
    
    return true;
}

std::vector<int32_t> PartitionManager::getEligiblePartitions(const std::string& table_name,
                                                            const void* condition) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto table_it = partitions_.find(table_name);
    if (table_it == partitions_.end()) {
        return {};
    }
    
    return selectPartitionsByCondition(table_name, condition);
}

std::shared_ptr<Partition> PartitionManager::getPartition(const std::string& table_name, 
                                                         int32_t partition_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto table_it = partitions_.find(table_name);
    if (table_it == partitions_.end()) {
        return nullptr;
    }
    
    auto partition_it = table_it->second.find(partition_id);
    if (partition_it == table_it->second.end()) {
        return nullptr;
    }
    
    return partition_it->second;
}

std::vector<std::shared_ptr<Partition>> PartitionManager::getAllPartitions(const std::string& table_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto table_it = partitions_.find(table_name);
    if (table_it == partitions_.end()) {
        return {};
    }
    
    std::vector<std::shared_ptr<Partition>> result;
    for (const auto& [partition_id, partition] : table_it->second) {
        result.push_back(partition);
    }
    
    return result;
}

std::vector<std::shared_ptr<PartitionMetadata>> PartitionManager::getPartitionMetadata(const std::string& table_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto table_it = partition_metadata_.find(table_name);
    if (table_it == partition_metadata_.end()) {
        return {};
    }
    
    std::vector<std::shared_ptr<PartitionMetadata>> result;
    for (const auto& [partition_id, metadata] : table_it->second) {
        result.push_back(std::shared_ptr<PartitionMetadata>(metadata.get()));
    }
    
    return result;
}

bool PartitionManager::updatePartitionStatistics(const std::string& table_name, int32_t partition_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Partition::PartitionStatistics stats;
    if (!collectPartitionStatistics(table_name, partition_id, stats)) {
        return false;
    }
    
    auto partition = getPartition(table_name, partition_id);
    if (partition) {
        partition->updateStatistics();
    }
    
    auto metadata_it = partition_metadata_[table_name].find(partition_id);
    if (metadata_it != partition_metadata_[table_name].end()) {
        metadata_it->second->row_count = stats.row_count;
        metadata_it->second->size_in_bytes = stats.data_size + stats.index_size;
        metadata_it->second->last_modified = stats.last_modified;
    }
    
    return true;
}

std::string PartitionManager::getPartitionStoragePath(const std::string& table_name, 
                                                     int32_t partition_id) const {
    std::ostringstream oss;
    oss << "data/partitions/" << table_name << "/partition_" << std::setfill('0') << std::setw(4) << partition_id;
    return oss.str();
}

void PartitionManager::notifyPartitionChange(const std::string& table_name,
                                           int32_t partition_id,
                                           const std::string& operation) {
    auto table_it = change_listeners_.find(table_name);
    if (table_it != change_listeners_.end()) {
        for (const auto& listener : table_it->second) {
            try {
                listener(table_name, partition_id, operation);
            } catch (const std::exception& e) {
                // 监听器异常不影响主流程
            }
        }
    }
}

std::vector<int32_t> PartitionManager::selectPartitionsByCondition(const std::string& table_name,
                                                                  const void* /*condition*/) const {
    std::vector<int32_t> eligible_partitions;
    
    auto table_it = partitions_.find(table_name);
    if (table_it == partitions_.end()) {
        return eligible_partitions;
    }
    
    // 简化的分区选择逻辑
    // 实际实现需要解析条件表达式，确定哪些分区可能包含满足条件的数据
    for (const auto& [partition_id, partition] : table_it->second) {
        // 这里应该根据分区类型和条件进行判断
        // 目前简化为返回所有活跃分区
        if (partition->getState() == Partition::State::ACTIVE) {
            eligible_partitions.push_back(partition_id);
        }
    }
    
    return eligible_partitions;
}

bool PartitionManager::collectPartitionStatistics(const std::string& /*table_name*/,
                                                 int32_t /*partition_id*/,
                                                 Partition::PartitionStatistics& stats) const {
    // 实际实现需要从存储层获取统计信息
    stats.row_count = 0;
    stats.data_size = 0;
    stats.index_size = 0;
    stats.last_analyzed = std::chrono::system_clock::now();
    stats.last_modified = std::chrono::system_clock::now();
    return true;
}

} // namespace storage
} // namespace sqlcc
