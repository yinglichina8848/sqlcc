#include "savepoint_manager.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {

// Savepoint implementation
Savepoint::Savepoint(const std::string& name, TransactionId txn_id, size_t undo_position)
    : name_(name), txn_id_(txn_id), undo_log_position_(undo_position),
      timestamp_(std::chrono::system_clock::now()) {}

Savepoint::~Savepoint() {}

void Savepoint::addLockedResource(const std::string& resource) {
    if (std::find(locked_resources_.begin(), locked_resources_.end(), resource) == locked_resources_.end()) {
        locked_resources_.push_back(resource);
    }
}

void Savepoint::removeLockedResource(const std::string& resource) {
    auto it = std::find(locked_resources_.begin(), locked_resources_.end(), resource);
    if (it != locked_resources_.end()) {
        locked_resources_.erase(it);
    }
}

bool Savepoint::hasLockedResource(const std::string& resource) const {
    return std::find(locked_resources_.begin(), locked_resources_.end(), resource) != locked_resources_.end();
}

// SavepointManager implementation
SavepointManager& SavepointManager::getInstance() {
    static SavepointManager instance;
    return instance;
}

SavepointManager::SavepointManager() = default;

SavepointManager::~SavepointManager() = default;

bool SavepointManager::createSavepoint(TransactionId txn_id, const std::string& savepoint_name) {
    if (savepoint_name.empty()) {
        last_error_ = "Savepoint name cannot be empty";
        return false;
    }

    // 检查保存点是否已存在
    if (savepointExists(txn_id, savepoint_name)) {
        last_error_ = "Savepoint '" + savepoint_name + "' already exists for transaction " + std::to_string(txn_id);
        return false;
    }

    // 创建新的保存点（这里简化为使用0作为undo位置，实际应从TransactionManager获取）
    auto savepoint = std::make_unique<Savepoint>(savepoint_name, txn_id, 0);
    savepoints_[txn_id].push_back(std::move(savepoint));

    return true;
}

bool SavepointManager::releaseSavepoint(TransactionId txn_id, const std::string& savepoint_name) {
    auto txn_it = savepoints_.find(txn_id);
    if (txn_it == savepoints_.end()) {
        last_error_ = "No savepoints found for transaction " + std::to_string(txn_id);
        return false;
    }

    auto& txn_savepoints = txn_it->second;

    // 从后往前查找保存点
    for (auto it = txn_savepoints.rbegin(); it != txn_savepoints.rend(); ++it) {
        if ((*it)->getName() == savepoint_name) {
            // 释放这个保存点之后的所有保存点
            size_t index = std::distance(txn_savepoints.begin(), it.base()) - 1;
            txn_savepoints.erase(txn_savepoints.begin() + index, txn_savepoints.end());
            return true;
        }
    }

    last_error_ = "Savepoint '" + savepoint_name + "' not found for transaction " + std::to_string(txn_id);
    return false;
}

bool SavepointManager::rollbackToSavepoint(TransactionId txn_id, const std::string& savepoint_name) {
    auto txn_it = savepoints_.find(txn_id);
    if (txn_it == savepoints_.end()) {
        last_error_ = "No savepoints found for transaction " + std::to_string(txn_id);
        return false;
    }

    auto& txn_savepoints = txn_it->second;

    // 查找保存点
    for (auto it = txn_savepoints.rbegin(); it != txn_savepoints.rend(); ++it) {
        if ((*it)->getName() == savepoint_name) {
            // 保留找到的保存点及其之前的保存点，删除之后的
            size_t index = std::distance(txn_savepoints.begin(), it.base()) - 1;
            txn_savepoints.erase(txn_savepoints.begin() + index + 1, txn_savepoints.end());
            return true;
        }
    }

    last_error_ = "Savepoint '" + savepoint_name + "' not found for transaction " + std::to_string(txn_id);
    return false;
}

std::shared_ptr<const Savepoint> SavepointManager::getSavepoint(TransactionId txn_id,
                                                               const std::string& savepoint_name) const {
    auto txn_it = savepoints_.find(txn_id);
    if (txn_it == savepoints_.end()) {
        return nullptr;
    }

    const auto& txn_savepoints = txn_it->second;
    for (const auto& savepoint : txn_savepoints) {
        if (savepoint->getName() == savepoint_name) {
            return std::shared_ptr<const Savepoint>(savepoint.get(),
                [](const Savepoint*) {}); // Empty deleter for const access
        }
    }

    return nullptr;
}

std::vector<std::shared_ptr<const Savepoint>> SavepointManager::getTransactionSavepoints(TransactionId txn_id) const {
    std::vector<std::shared_ptr<const Savepoint>> result;

    auto txn_it = savepoints_.find(txn_id);
    if (txn_it != savepoints_.end()) {
        const auto& txn_savepoints = txn_it->second;
        for (const auto& savepoint : txn_savepoints) {
            result.push_back(std::shared_ptr<const Savepoint>(savepoint.get(),
                [](const Savepoint*) {})); // Empty deleter for const access
        }
    }

    return result;
}

bool SavepointManager::savepointExists(TransactionId txn_id, const std::string& savepoint_name) const {
    return getSavepoint(txn_id, savepoint_name) != nullptr;
}

void SavepointManager::clearTransactionSavepoints(TransactionId txn_id) {
    auto it = savepoints_.find(txn_id);
    if (it != savepoints_.end()) {
        savepoints_.erase(it);
    }
}

const std::string& SavepointManager::getLastError() const {
    return last_error_;
}

} // namespace sqlcc
