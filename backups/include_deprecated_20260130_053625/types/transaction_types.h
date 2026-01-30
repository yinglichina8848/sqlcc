/**
 * @file transaction_types.h
 * @brief SQLCC事务相关基础类型定义
 */

#ifndef SQLCC_TRANSACTION_TYPES_H
#define SQLCC_TRANSACTION_TYPES_H

#include <chrono>
#include <cstdint>

namespace sqlcc {

/**
 * @brief 事务ID类型 - 唯一标识一个事务
 *
 * WHY层 - 设计意图：
 *   TransactionId是数据库事务的核心标识符，用于跟踪和管理事务的生命周期。
 *   通过时间戳和ID的组合，确保事务ID的全局唯一性和可排序性。
 *
 * WHAT层 - 功能说明：
 *   - 封装事务ID和创建时间戳
 *   - 提供事务的唯一标识和排序能力
 *   - 支持事务生命周期管理
 *
 * HOW层 - 实现机制：
 *   - 使用64位整数作为事务ID
 *   - 使用系统时钟作为时间戳
 *   - 提供构造函数和比较操作符
 */
class TransactionId {
public:
    /**
     * 默认构造函数
     */
    TransactionId() : id_(0), timestamp_(std::chrono::system_clock::now()) {}

    /**
     * 参数化构造函数
     * @param id 事务ID
     * @param timestamp 创建时间戳
     */
    TransactionId(uint64_t id, std::chrono::system_clock::time_point timestamp)
        : id_(id), timestamp_(timestamp) {}

    /**
     * 获取事务ID
     * @return 事务ID
     */
    uint64_t get_id() const { return id_; }

    /**
     * 获取创建时间戳
     * @return 创建时间戳
     */
    std::chrono::system_clock::time_point get_timestamp() const { return timestamp_; }

    /**
     * 比较操作符 - 用于事务排序
     */
    bool operator==(const TransactionId& other) const {
        return id_ == other.id_ && timestamp_ == other.timestamp_;
    }

    bool operator!=(const TransactionId& other) const {
        return !(*this == other);
    }

    bool operator<(const TransactionId& other) const {
        return timestamp_ < other.timestamp_;
    }

    bool operator<=(const TransactionId& other) const {
        return timestamp_ <= other.timestamp_;
    }

    bool operator>(const TransactionId& other) const {
        return timestamp_ > other.timestamp_;
    }

    bool operator>=(const TransactionId& other) const {
        return timestamp_ >= other.timestamp_;
    }

private:
    uint64_t id_;                                    ///< 事务ID
    std::chrono::system_clock::time_point timestamp_; ///< 创建时间戳
};

/**
 * @brief 锁类型枚举 - 定义数据库锁的类型
 *
 * WHY层 - 设计意图：
 *   锁类型是并发控制的基础，定义了不同级别的访问权限。
 *   通过明确的锁类型定义，实现精确的并发控制和冲突检测。
 *
 * WHAT层 - 锁类型说明：
 *   - SHARED: 共享锁，允许多个事务同时读取
 *   - EXCLUSIVE: 排他锁，保证独占访问
 *
 * HOW层 - 实现机制：
 *   - SHARED锁：读操作使用，允许多个并发读
 *   - EXCLUSIVE锁：写操作使用，保证独占访问
 *   - 兼容性矩阵：定义锁类型间的兼容关系
 */
enum class LockType {
    SHARED,     ///< 共享锁：允许多个事务同时读取
    EXCLUSIVE   ///< 排他锁：保证独占访问，阻止其他事务
};

/**
 * @brief 锁模式枚举 - 更详细的锁模式定义（扩展用）
 *
 * WHY层 - 设计意图：
 *   提供更丰富的锁模式选择，支持复杂的并发控制策略。
 *   为未来扩展提供基础，如意向锁、更新锁等。
 *
 * WHAT层 - 锁模式说明：
 *   - SHARED: 共享锁
 *   - EXCLUSIVE: 排他锁
 *   - INTENTION_SHARED: 意向共享锁
 *   - INTENTION_EXCLUSIVE: 意向排他锁
 *   - SHARED_INTENTION_EXCLUSIVE: 共享意向排他锁
 *   - UPDATE: 更新锁
 */
enum class LockMode {
    SHARED,                         ///< 共享锁
    EXCLUSIVE,                      ///< 排他锁
    INTENTION_SHARED,              ///< 意向共享锁
    INTENTION_EXCLUSIVE,           ///< 意向排他锁
    SHARED_INTENTION_EXCLUSIVE,    ///< 共享意向排他锁
    UPDATE                         ///< 更新锁
};

} // namespace sqlcc

#endif // SQLCC_TRANSACTION_TYPES_H
