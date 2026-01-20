/**
 * @file storage_engine.cpp
 *
 * WHY: 为什么需要存储引擎？
 *
 * 数据库系统需要一个专门的存储子系统来管理数据的持久化、缓存、索引和并发访问。没有存储引擎，数据就无法在内存和磁盘之间高效地传输，系统就无法提供ACID事务特性和高性能的数据访问。存储引擎是数据库系统的核心基础设施，直接决定了系统的性能、可扩展性和可靠性。
 *
 * 主要问题解决：
 * 1. 数据持久化：确保数据在系统重启后仍然可用
 * 2. 内存管理：高效地在内存和磁盘之间交换数据
 * 3. 索引管理：提供快速的数据查找和排序能力
 * 4. 并发控制：允许多个事务同时安全地访问数据
 * 5. 故障恢复：从系统崩溃中恢复数据一致性
 *
 * 存储引擎失败的影响：
 * - 数据丢失：无法保证数据的持久化存储
 * - 性能低下：所有数据操作都需要直接访问磁盘
 * - 并发冲突：多个用户无法同时访问数据
 * - 系统不稳定：缺乏故障恢复和数据一致性保证
 *
 * WHAT: 这实现了什么功能？
 *
 * 存储引擎提供完整的数据库存储管理功能：
 * - 页面管理：数据的物理存储单位和内存缓冲
 * - 磁盘I/O：数据在内存和磁盘之间的传输
 * - 缓冲池：高速缓存以减少磁盘访问
 * - 索引系统：B+树索引实现快速数据查找
 * - 并发控制：锁管理器处理多事务并发访问
 * - 事务日志：WAL（Write-Ahead Logging）确保ACID特性
 * - 故障恢复：从崩溃中恢复数据一致性
 *
 * 核心组件：
 * - StorageEngine：存储引擎主类，协调所有存储操作
 * - BufferPoolSharded：分片缓冲池，管理内存中的数据页面
 * - DiskManager：磁盘管理器，处理物理文件I/O
 * - IndexManager：索引管理器，维护B+树索引结构
 * - Page：数据页面，存储引擎的基本存储单位
 * - WAL：预写日志系统，确保事务持久性
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 页面架构：使用固定大小的页面作为存储基本单位
 * 2. 缓冲池：LRU缓存策略管理内存中的页面
 * 3. 分片设计：缓冲池分片减少锁竞争，提高并发性
 * 4. 预写日志：事务提交前先写日志，确保持久性
 * 5. B+树索引：平衡多叉树实现高效的范围查询
 * 6. 锁管理：两阶段锁协议控制并发访问
 * 7. 故障恢复：ARIES算法从日志中重放事务
 *
 * 架构设计：
 * - 分层架构：缓冲层、文件层、磁盘层的清晰分离
 * - 抽象接口：统一的存储接口支持不同存储后端
 * - 插件架构：可扩展的索引类型和存储策略
 * - 异步处理：后台线程处理日志写入和页面刷新
 * - 配置驱动：运行时配置缓冲池大小、索引参数等
 *
 * 性能优化：
 * - 预取策略：预测性加载数据页面到内存
 * - 批量操作：合并多个小的I/O操作
 * - 内存预分配：预先分配缓冲池内存减少运行时开销
 * - 索引优化：自适应索引维护和查询优化
 * - 并发优化：无锁数据结构和细粒度锁
 *
 * @note 该实现专为SQLCC数据库系统优化，支持高并发和ACID事务特性
 * @see include/storage_engine.h
 */

#include "include/storage_engine.h"
#include "include/storage_engine/index_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace sqlcc {

StorageEngine::StorageEngine(ConfigManager& config_manager, const std::string& db_path)
    : config_manager_(config_manager), db_path_(db_path) {
    // 验证输入参数
    if (db_path.empty()) {
        throw std::invalid_argument("Database path cannot be empty");
    }

    // 创建数据库文件路径
    std::string db_file = db_path_ + "/sqlcc.db";
    try {
        // 确保数据库目录存在
        std::filesystem::create_directories(db_path_);
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Failed to create database directory: " + std::string(e.what()));
    }

    // 创建磁盘管理器
    disk_manager_ = std::make_unique<DiskManager>(db_file, config_manager_);

    // 获取缓冲池配置，使用更安全的配置获取方式
    size_t buffer_pool_size = config_manager_.GetInt("buffer.pool.size", 64);
    if (buffer_pool_size == 0) {
        buffer_pool_size = 64;  // 设置最小缓冲池大小
    }

    size_t shard_count = config_manager_.GetInt("buffer.shard.count", 16);
    if (shard_count == 0) {
        shard_count = 4;  // 设置最小分片数量
    }

    // 创建缓冲池
    buffer_pool_ = std::make_unique<BufferPoolSharded>(std::move(disk_manager_), config_manager_, buffer_pool_size, shard_count);

    // 注意：不能在这里创建索引管理器，因为shared_from_this()只能在对象完全构造后使用
    // 索引管理器的创建将延迟到首次访问时
    index_manager_ = nullptr;
}

StorageEngine::~StorageEngine() {
    // 析构函数实现 - 刷新所有页面到磁盘
    if (buffer_pool_) {
        buffer_pool_->FlushAllPages();
    } else {
        // 确保buffer_pool_为空时的安全处理
    }
}

// 添加一个方法来初始化索引管理器
void StorageEngine::InitializeIndexManager() {
    if (!index_manager_) {
        index_manager_ = std::make_unique<IndexManager>(shared_from_this(), config_manager_);
    }
}

std::unique_ptr<Page> StorageEngine::NewPage(int32_t *page_id) {
    // 创建新页面的实现
    if (!buffer_pool_) {
        return nullptr;
    }
    
    int32_t new_page_id;
    auto page_ptr = buffer_pool_->NewPage(&new_page_id);
    
    if (page_id != nullptr) {
        *page_id = new_page_id;
    }
    
    return page_ptr;
}

std::shared_ptr<Page> StorageEngine::FetchPage(int32_t page_id) {
    // 获取页面的实现
    if (!buffer_pool_) {
        return nullptr;
    }
    
    return buffer_pool_->FetchPage(page_id);
}

bool StorageEngine::UnpinPage(int32_t page_id, bool is_dirty) {
    // 取消固定页面的实现
    if (!buffer_pool_) {
        return false;
    }
    
    return buffer_pool_->UnpinPage(page_id, is_dirty);
}

bool StorageEngine::FlushPage(int32_t page_id) {
    // 刷新页面到磁盘的实现
    if (!buffer_pool_) {
        return false;
    }
    
    return buffer_pool_->FlushPage(page_id);
}

bool StorageEngine::DeletePage(int32_t page_id) {
    // 删除页面的实现
    if (!buffer_pool_) {
        return false;
    }
    
    return buffer_pool_->DeletePage(page_id);
}

void StorageEngine::FlushAllPages() {
    // 刷新所有页面到磁盘的实现
    if (buffer_pool_) {
        buffer_pool_->FlushAllPages();
    }
}

std::string StorageEngine::GetStats() const {
    // 获取统计信息的实现
    if (!buffer_pool_) {
        return "Storage Engine Stats: Not initialized";
    }
    
    auto stats = buffer_pool_->GetStats();
    std::ostringstream oss;
    oss << "Storage Engine Stats:\n";
    oss << "  Total Accesses: " << stats.at("total_accesses") << "\n";
    oss << "  Total Hits: " << stats.at("total_hits") << "\n";
    oss << "  Hit Rate: " << stats.at("hit_rate") << "\n";
    oss << "  Used Pages: " << stats.at("used_pages") << "\n";
    oss << "  Pool Size: " << stats.at("pool_size");
    
    return oss.str();
}

} // namespace sqlcc
