#ifndef SQLCC_TRANSACTION_SAVEPOINT_MANAGER_H
#define SQLCC_TRANSACTION_SAVEPOINT_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>

namespace sqlcc {

using TransactionId = uint64_t;

/**
 * @brief 保存点信息
 */
class Savepoint {
public:
    Savepoint(const std::string& name, TransactionId txn_id, size_t undo_position);

    ~Savepoint();

    // Getters
    const std::string& getName() const { return name_; }
    TransactionId getTransactionId() const { return txn_id_; }
    size_t getUndoLogPosition() const { return undo_log_position_; }
    const std::chrono::system_clock::time_point& getTimestamp() const { return timestamp_; }

    // 锁资源管理
    void addLockedResource(const std::string& resource);
    void removeLockedResource(const std::string& resource);
    bool hasLockedResource(const std::string& resource) const;
    const std::vector<std::string>& getLockedResources() const { return locked_resources_; }

private:
    std::string name_;
    TransactionId txn_id_;
    size_t undo_log_position_;
    std::chrono::system_clock::time_point timestamp_;
    std::vector<std::string> locked_resources_;
};

/**
 * @brief 保存点管理器
 *
 * 负责事务保存点的创建、管理和回滚操作
 */
class SavepointManager {
public:
    static SavepointManager& getInstance();

    /**
     * 创建保存点
     * @param txn_id 事务ID
     * @param savepoint_name 保存点名称
     * @return 是否成功
     */
    bool createSavepoint(TransactionId txn_id, const std::string& savepoint_name);

    /**
     * 释放保存点
     * @param txn_id 事务ID
     * @param savepoint_name 保存点名称
     * @return 是否成功
     */
    bool releaseSavepoint(TransactionId txn_id, const std::string& savepoint_name);

    /**
     * 回滚到保存点
     * @param txn_id 事务ID
     * @param savepoint_name 保存点名称
     * @return 是否成功
     */
    bool rollbackToSavepoint(TransactionId txn_id, const std::string& savepoint_name);

    /**
     * 获取保存点信息
     * @param txn_id 事务ID
     * @param savepoint_name 保存点名称
     * @return 保存点智能指针，如果不存在返回nullptr
     */
    std::shared_ptr<const Savepoint> getSavepoint(TransactionId txn_id,
                                                 const std::string& savepoint_name) const;

    /**
     * 获取事务的所有保存点
     * @param txn_id 事务ID
     * @return 保存点列表
     */
    std::vector<std::shared_ptr<const Savepoint>> getTransactionSavepoints(TransactionId txn_id) const;

    /**
     * 检查保存点是否存在
     * @param txn_id 事务ID
     * @param savepoint_name 保存点名称
     * @return 是否存在
     */
    bool savepointExists(TransactionId txn_id, const std::string& savepoint_name) const;

    /**
     * 清理事务的所有保存点
     * @param txn_id 事务ID
     */
    void clearTransactionSavepoints(TransactionId txn_id);

    /**
     * 获取最后错误信息
     */
    const std::string& getLastError() const;

private:
    SavepointManager();
    ~SavepointManager();

    // 禁用拷贝
    SavepointManager(const SavepointManager&) = delete;
    SavepointManager& operator=(const SavepointManager&) = delete;

    std::unordered_map<TransactionId, std::vector<std::unique_ptr<Savepoint>>> savepoints_;
    mutable std::string last_error_;
};

} // namespace sqlcc

#endif // SQLCC_TRANSACTION_SAVEPOINT_MANAGER_H
