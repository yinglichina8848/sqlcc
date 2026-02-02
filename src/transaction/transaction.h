/**
 * @file transaction.h
 * @brief SQLCC事务抽象接口 - 数据库ACID特性的协议基础
 *
 * Transaction 类定义了数据库事务的通用行为契约。它是实现原子性（Atomicity）和
 * 一致性（Consistency）的核心抽象。所有具体的事务实现（如单机事务、分布式事务）
 * 都必须遵循此接口定义的生命周期。
 *
 * 📚 配套教材参考：
 * - [第8章：OLTP事务处理](../../textbook/《数据库系统原理与开发实践》.md#第八章oltp事务处理)
 * - [8.1 事务的定义与ACID属性](../../textbook/《数据库系统原理与开发实践》.md#81-事务的定义与acid属性)
 * - [8.2 事务控制语句实现](../../textbook/《数据库系统原理与开发实践》.md#82-事务控制语句实现)
 *
 * WHY层 - 设计意图：
 *   1. **抽象屏障**：执行层无需关心事务的具体实现细节（是基于WAL还是基于影子页），只需调用接口。
 *   2. **一致性契约**：确保所有数据修改操作都在明确的事务边界（Begin/Commit/Rollback）内进行。
 *   3. **扩展性**：支持未来引入XA事务、两阶段提交（2PC）等高级事务协议。
 *
 * WHAT层 - 功能说明：
 *   - 生命周期管理：提供事务的开始、提交和回滚接口。
 *   - 状态查询：通过 IsActive() 实时追踪事务执行状态。
 *   - 唯一标识：GetId() 提供全局唯一的事务标识，用于死锁检测和日志关联。
 *
 * HOW层 - 协作机制：
 *   - **原子性保证**：如果 Commit() 失败或显式调用 Rollback()，系统必须撤销该事务的所有变更。
 *   - **隔离性协作**：事务对象通常持有隔离级别信息（隔离性由 TransactionManager 配合锁实现）。
 *   - **持久性对接**：Commit() 操作必须等待日志写入物理磁盘。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#pragma once

#include <string>
#include <memory>

namespace sqlcc {

/**
 * @class Transaction
 * @brief 事务抽象基类 - 定义了事务的生命周期和操作契约
 */
class Transaction {
public:
    virtual ~Transaction() = default;

    /**
     * @brief 启动事务
     * 
     * WHY: 为后续的操作分配资源（如事务ID、Undo空间），建立一致性视图快照。
     * @return bool 成功启动返回 true
     */
    virtual bool Begin() = 0;

    /**
     * @brief 提交事务
     * 
     * WHY: 将当前事务的所有内存修改永久持久化到磁盘。
     * WHAT: 遵循 WAL 协议，释放持有的所有锁。
     * @return bool 提交成功返回 true；如果因冲突被强制回滚则返回 false
     */
    virtual bool Commit() = 0;

    /**
     * @brief 回滚事务
     * 
     * WHY: 在遇到异常、死锁或用户主动取消时，撤销事务的所有影响，恢复数据库到事务开始前的状态。
     */
    virtual bool Rollback() = 0;

    /**
     * @brief 检查事务是否处于活跃状态
     */
    virtual bool IsActive() const = 0;

    /**
     * @brief 获取事务全局唯一标识符
     * 常用于死锁检测（Wait-for Graph）和日志追踪。
     */
    virtual std::string GetId() const = 0;
};

/**
 * @class TransactionFactory
 * @brief 事务工厂接口 - 遵循工厂模式设计
 * 
 * WHY: 解耦事务的创建逻辑与使用逻辑，支持不同隔离级别的事务创建。
 */
class TransactionFactory {
public:
    virtual ~TransactionFactory() = default;

    /**
     * @brief 创建一个新的事务实例
     */
    virtual std::unique_ptr<Transaction> CreateTransaction() = 0;
};

} // namespace sqlcc

