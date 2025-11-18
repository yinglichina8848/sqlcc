#ifndef SQLCC_DATABASE_MANAGER_H
#define SQLCC_DATABASE_MANAGER_H

#include <memory>
#include <string>

#include "buffer_pool_sharded.h"
#include "transaction_manager.h"

namespace sqlcc {

/**
 * 数据库管理器
 * 集成了shard化BufferPool和striped key锁，提供高并发的缓存和事务支持
 */
class DatabaseManager {
public:
    /**
     * 构造函数
     * @param db_path 数据库路径
     * @param buffer_pool_size 缓冲池大小（页面数量）
     * @param shard_count 缓冲池shard数量（建议为2的幂）
     * @param stripe_count 键锁stripe数量（建议为2的幂）
     */
    DatabaseManager(const std::string& db_path,
                   size_t buffer_pool_size = 1024,
                   size_t shard_count = 16,
                   size_t stripe_count = 64);

    /**
     * 析构函数
     */
    ~DatabaseManager();

    /**
     * 获取缓冲池实例
     * @return 缓冲池指针
     */
    std::shared_ptr<ShardedBufferPool> GetBufferPool() {
        return buffer_pool_;
    }

    /**
     * 获取事务管理器实例
     * @return 事务管理器指针
     */
    std::shared_ptr<TransactionManager> GetTransactionManager() {
        return txn_manager_;
    }

    /**
     * 开启事务
     * @param isolation_level 隔离级别
     * @return 事务ID
     */
    TransactionId BeginTransaction(IsolationLevel isolation_level = IsolationLevel::SNAPSHOT_ISOLATION);

    /**
     * 提交事务
     * @param txn_id 事务ID
     * @return 是否提交成功
     */
    bool CommitTransaction(TransactionId txn_id);

    /**
     * 回滚事务
     * @param txn_id 事务ID
     * @return 是否回滚成功
     */
    bool RollbackTransaction(TransactionId txn_id);

    /**
     * 事务中读取页面
     * @param txn_id 事务ID
     * @param page_id 页面ID
     * @param page 输出参数，返回页面指针
     * @return 是否读取成功
     */
    bool ReadPage(TransactionId txn_id, page_id_t page_id, Page** page);

    /**
     * 事务中写入页面
     * @param txn_id 事务ID
     * @param page_id 页面ID
     * @param page 页面指针
     * @return 是否写入成功
     */
    bool WritePage(TransactionId txn_id, page_id_t page_id, Page* page);

    /**
     * 事务中锁定键
     * @param txn_id 事务ID
     * @param key 键
     * @return 是否锁定成功
     */
    bool LockKey(TransactionId txn_id, const std::string& key);

    /**
     * 事务中解锁键
     * @param txn_id 事务ID
     * @param key 键
     * @return 是否解锁成功
     */
    bool UnlockKey(TransactionId txn_id, const std::string& key);

    /**
     * 刷新所有脏页到磁盘
     * @return 是否刷新成功
     */
    bool FlushAllPages();

    /**
     * 关闭数据库
     * @return 是否关闭成功
     */
    bool Close();

private:
    std::shared_ptr<ShardedBufferPool> buffer_pool_;     // shard化缓冲池
    std::shared_ptr<TransactionManager> txn_manager_;    // 事务管理器
    std::string db_path_;                               // 数据库路径
    bool is_closed_;                                    // 是否已关闭
};

}  // namespace sqlcc

#endif  // SQLCC_DATABASE_MANAGER_H