/**
 * @file transaction_types.h
 * @brief SQLCC事务系统基础类型定义
 *
 * 本文件定义了事务管理系统中使用的核心数据类型和枚举。
 * 这些类型构成了事务调度、并发控制和死锁检测的基础。
 *
 * 📚 配套教材参考：
 * - [第8章：OLTP事务处理](../../textbook/《数据库系统原理与开发实践》.md#第八章oltp事务处理)
 * - [9.2 两阶段锁协议](../../textbook/《数据库系统原理与开发实践》.md#92-两阶段锁协议与死锁处理)
 *
 * WHY层 - 设计意图：
 *   1. **类型安全**：通过封装 `TransactionId` 类而不是简单的 `uint64_t`，防止与普通整数混淆。
 *   2. **可排序性**：事务ID必须单调递增，以便在 MVCC 中确定可见性顺序。
 *   3. **粒度控制**：`LockMode` 提供了比简单的读/写锁更丰富的语义（如意向锁），支持多粒度锁定协议。
 */

#ifndef SQLCC_TRANSACTION_TYPES_H
#define SQLCC_TRANSACTION_TYPES_H

#include <chrono>
#include <cstdint>

namespace sqlcc {

/**
 * @class TransactionId
 * @brief 事务全局唯一标识符
 *
 * TransactionId 不仅是一个唯一ID，还隐含了时间顺序信息。
 * 在 MVCC 实现中，事务 ID 的大小关系决定了数据的可见性。
 */
class TransactionId {
public:
    /**
     * @brief 默认构造函数
     * 初始化为无效事务ID（0）和当前时间。
     */
    TransactionId() : id_(0), timestamp_(std::chrono::system_clock::now()) {}

    /**
     * @brief 构造函数
     * @param id 分配的事务序列号
     * @param timestamp 事务开始时间
     */
    TransactionId(uint64_t id, std::chrono::system_clock::time_point timestamp)
        : id_(id), timestamp_(timestamp) {}

    /**
     * @brief 获取数字ID
     */
    uint64_t get_id() const { return id_; }

    /**
     * @brief 获取事务开始时间戳
     * 用于死锁检测中的超时判断或牺牲者选择策略（如 Wait-Die / Wound-Wait）。
     */
    std::chrono::system_clock::time_point get_timestamp() const { return timestamp_; }

    // --- 比较操作符 ---
    // 事务ID的比较对于 MVCC 版本链遍历至关重要

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
    uint64_t id_;                                    ///< 单调递增的事务序列号
    std::chrono::system_clock::time_point timestamp_; ///< 事务创建时的物理时间
};

/**
 * @enum LockType
 * @brief 基础锁类型 - 互斥性定义
 *
 * 定义了最基本的锁互斥关系：共享（S）与排他（X）。
 */
enum class LockType {
    SHARED,     ///< 共享锁 (S)：允许并发读，阻止并发写
    EXCLUSIVE   ///< 排他锁 (X)：独占访问，阻止任何并发操作
};

/**
 * @enum LockMode
 * @brief 扩展锁模式 - 支持多粒度锁定协议 (MGL)
 *
 * 为了在表级和行级之间高效地管理锁，引入了意向锁（Intention Locks）。
 * 意向锁表示事务"打算"在更细的粒度上获取锁。
 *
 * 兼容性矩阵：
 *       IS   IX   S    SIX  X
 * IS    Yes  Yes  Yes  Yes  No
 * IX    Yes  Yes  No   No   No
 * S     Yes  No   Yes  No   No
 * SIX   Yes  No   No   No   No
 * X     No   No   No   No   No
 */
enum class LockMode {
    SHARED,                         ///< S 锁：锁定当前节点及所有后代节点（读）
    EXCLUSIVE,                      ///< X 锁：锁定当前节点及所有后代节点（写）
    INTENTION_SHARED,              ///< IS 锁：打算在后代节点上加 S 锁
    INTENTION_EXCLUSIVE,           ///< IX 锁：打算在后代节点上加 X 锁
    SHARED_INTENTION_EXCLUSIVE,    ///< SIX 锁：S + IX，锁定当前节点（读），且打算在后代加 X 锁
    UPDATE                         ///< U 锁：更新锁，用于防止死锁的升级锁（预留）
};

} // namespace sqlcc

#endif // SQLCC_TRANSACTION_TYPES_H

