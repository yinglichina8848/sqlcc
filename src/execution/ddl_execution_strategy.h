/**
 * WHY: 为什么需要专门的DDL执行策略？
 *
 * DDL语句是数据库系统的"建筑师"，负责定义和修改数据库结构，
 * 传统方案存在诸多技术挑战：
 * - DDL操作的原子性保证：DDL语句需要确保操作的原子性，不能部分成功
 * - 并发DDL操作的协调：多用户同时执行DDL操作时的冲突处理
 * - DDL操作的权限控制：严格的权限检查和访问控制机制
 * - DDL操作的回滚处理：DDL语句执行失败时的清理和回滚策略
 * - DDL操作的性能影响：DDL操作对系统性能的实时影响控制
 * - DDL操作的元数据管理：系统目录和元数据的更新维护
 * - DDL操作的事务隔离：DDL操作与其他事务的隔离级别控制
 *
 * DDL执行策略的核心价值：
 * 1. DDL操作的原子性保障：确保DDL操作要么完全成功，要么完全失败
 * 2. 并发控制和协调：安全处理多用户并发DDL操作的冲突
 * 3. 严格的权限验证：基于角色的细粒度权限控制机制
 * 4. 智能回滚和恢复：DDL操作失败时的自动清理和状态恢复
 * 5. 性能影响最小化：控制DDL操作对系统性能的影响范围
 * 6. 元数据一致性维护：确保系统目录和元数据的强一致性
 * 7. 事务隔离控制：适当的隔离级别确保数据操作的安全性
 *
 * 🏗️ 设计模式：策略模式(Strategy Pattern) + 模板方法模式(Template Method Pattern) + 命令模式(Command Pattern)
 *
 * DDL执行策略作为策略模式的应用：
 * - 算法封装：将不同DDL语句的处理算法封装在独立的策略中
 * - 运行时选择：根据DDL语句类型动态选择合适的执行策略
 * - 算法替换：可以透明地替换或扩展新的DDL处理算法
 * - 代码复用：避免重复实现相似的DDL处理逻辑
 * - 易于测试：每个DDL处理策略都可以独立测试
 *
 * SOLID原则体现：
 * - 单一职责：DDL执行策略专门负责数据定义语言语句的执行和管理
 * - 开闭原则：新DDL语句类型通过扩展现有类实现
 * - 里氏替换：不同DDL实现可以互相替换
 * - 接口隔离：DDL接口精确定义数据定义契约
 * - 依赖倒置：执行器依赖抽象的存储和目录接口
 *
 * WHAT: DDL执行策略系统 - 数据定义语言统一执行框架
 *
 * 核心功能：
 * - DDL语句类型支持：支持CREATE、DROP、ALTER等DDL语句类型
 * - 原子性操作保障：确保DDL操作的原子性和一致性
 * - 并发控制协调：多用户并发DDL操作的安全协调
 * - 权限验证机制：基于角色的细粒度权限控制
 * - 回滚和恢复处理：DDL操作失败时的状态恢复
 * - 元数据管理维护：系统目录和元数据的更新维护
 * - 性能监控统计：DDL执行过程的性能监控和统计
 *
 * 系统组件：
 * - DDLExecutionStrategy：核心执行策略，协调整个DDL操作过程
 * - TableDDLHandler：表DDL处理器，处理表相关的DDL操作
 * - IndexDDLHandler：索引DDL处理器，处理索引相关的DDL操作
 * - ConstraintDDLHandler：约束DDL处理器，处理约束相关的DDL操作
 * - PermissionValidator：权限验证器，验证DDL操作的权限
 * - MetadataManager：元数据管理器，管理系统目录信息
 * - TransactionCoordinator：事务协调器，协调DDL操作的事务
 *
 * DDL语句类型支持：
 * - CREATE TABLE：创建新表的DDL语句，支持各种列定义和约束
 * - DROP TABLE：删除现有表的DDL语句，支持级联删除选项
 * - ALTER TABLE：修改表结构的DDL语句，支持添加、删除、修改列
 * - CREATE INDEX：创建索引的DDL语句，支持不同类型的索引
 * - DROP INDEX：删除索引的DDL语句，支持索引的重建和优化
 * - TRUNCATE TABLE：清空表的DDL语句，快速删除表中所有数据
 * - RENAME TABLE：重命名表的DDL语句，支持表的重命名操作
 *
 * 原子性保障机制：
 * - 事务封装：将DDL操作封装在事务中执行
 * - 两阶段提交：使用两阶段提交协议确保操作原子性
 * - 补偿操作：失败时执行补偿操作恢复原始状态
 * - 状态检查：在执行前验证系统状态的一致性
 * - 锁机制：使用适当的锁机制防止并发冲突
 * - 回滚策略：完善的回滚策略处理执行失败的情况
 *
 * 并发控制机制：
 * - 共享锁和排他锁：根据操作类型使用不同的锁机制
 * - 锁升级策略：动态调整锁的粒度和范围
 * - 死锁检测：主动检测和解决死锁情况
 * - 超时控制：防止长时间持有锁导致的系统阻塞
 * - 队列管理：公平的锁请求队列管理
 * - 冲突解决：智能解决并发DDL操作的冲突
 *
 * 权限验证系统：
 * - 用户角色识别：识别当前用户的角色和权限级别
 * - 对象所有权验证：验证用户对数据库对象的操作权限
 * - 权限继承机制：支持权限的继承和委托机制
 * - 权限缓存优化：缓存权限检查结果提高性能
 * - 审计日志记录：记录所有DDL操作的权限检查结果
 * - 权限策略配置：灵活配置的权限策略和规则
 *
 * 回滚和恢复机制：
 * - 操作日志记录：详细记录DDL操作的每一步骤
 * - 逆操作生成：为每个DDL操作生成对应的逆操作
 * - 状态快照保存：保存执行前的系统状态快照
 * - 自动回滚执行：失败时自动执行回滚操作
 * - 手动干预接口：提供手动干预的接口和工具
 * - 恢复验证检查：验证回滚操作的正确性和完整性
 *
 * 元数据管理机制：
 * - 系统目录更新：实时更新系统目录信息
 * - 元数据一致性：确保元数据的强一致性
 * - 缓存同步更新：同步更新各级缓存中的元数据
 * - 版本控制管理：元数据的版本控制和历史记录
 * - 依赖关系跟踪：跟踪对象间的依赖关系
 * - 元数据验证：验证元数据的完整性和正确性
 *
 * 执行流程：
 * - 语法解析验证：解析DDL语句并验证语法正确性
 * - 权限预检查：在执行前验证用户的操作权限
 * - 元数据锁定：锁定相关的元数据防止并发冲突
 * - 依赖关系检查：检查对象间的依赖关系和约束
 * - 执行准备阶段：准备执行环境和必要资源
 * - 原子性执行：在一个事务中执行DDL操作
 * - 元数据更新：更新系统目录和元数据信息
 * - 缓存清理同步：清理和同步各级缓存
 * - 结果返回确认：返回执行结果并确认操作成功
 *
 * 性能优化策略：
 * - 操作批处理：将多个DDL操作批量执行
 * - 索引优化延迟：延迟索引创建提高性能
 * - 并行执行支持：支持DDL操作的并行执行
 * - 资源预分配：预分配必要的系统资源
 * - 执行计划优化：优化DDL操作的执行计划
 * - 监控和调优：实时监控和调优DDL性能
 *
 * 内存管理策略：
 * - 临时对象管理：有效管理DDL执行过程中的临时对象
 * - 内存使用监控：监控DDL操作的内存使用情况
 * - 垃圾回收优化：优化临时对象的回收策略
 * - 内存池使用：使用内存池减少分配开销
 * - 大对象处理：特殊处理大对象的内存管理
 *
 * 接口设计：
 * - 执行接口：DDL语句的主要执行接口
 * - 验证接口：DDL语句的验证和检查接口
 * - 权限接口：DDL操作的权限验证接口
 * - 监控接口：DDL执行的监控和统计接口
 * - 扩展接口：新DDL类型的扩展接口
 *
 * HOW: DDL执行策略系统的实现机制
 *
 * 策略模式实现：
 * 1. 抽象策略基类：定义DDL执行的通用接口和行为
 * 2. 具体策略实现：CREATE、DROP、ALTER等具体DDL策略
 * 3. 策略选择器：根据DDL语句类型选择合适的执行策略
 * 4. 上下文管理：维护DDL执行的上下文和状态
 * 5. 结果封装：统一的DDL结果封装和返回
 *
 * 原子性保障实现：
 * 1. 事务封装：将DDL操作封装在单独的事务中
 * 2. 保存点设置：在关键节点设置事务保存点
 * 3. 异常捕获：捕获执行过程中的各种异常
 * 4. 回滚执行：异常发生时执行回滚操作
 * 5. 资源清理：确保资源在异常情况下正确清理
 * 6. 状态恢复：恢复到执行前的系统状态
 *
 * 并发控制实现：
 * 1. 锁获取策略：根据操作类型获取适当的锁
 * 2. 锁持有时间：最小化锁的持有时间
 * 3. 锁升级机制：动态升级锁的粒度
 * 4. 死锁预防：使用锁顺序和超时机制预防死锁
 * 5. 锁释放策略：及时释放不再需要的锁
 * 6. 冲突解决：智能解决锁冲突的情况
 *
 * 权限验证实现：
 * 1. 用户身份识别：识别当前操作用户的身份
 * 2. 权限规则加载：加载适用的权限规则和策略
 * 3. 权限计算评估：计算用户对特定操作的权限
 * 4. 权限缓存利用：利用缓存提高权限检查性能
 * 5. 权限审计记录：记录权限检查的结果和过程
 * 6. 权限策略更新：动态更新权限策略和规则
 *
 * 元数据管理实现：
 * 1. 元数据读取：从系统目录读取当前的元数据
 * 2. 元数据修改：根据DDL操作修改元数据
 * 3. 一致性保证：确保元数据修改的原子性
 * 4. 缓存同步：同步各级缓存中的元数据
 * 5. 依赖更新：更新依赖于修改对象的元数据
 * 6. 版本记录：记录元数据的版本变更历史
 *
 * 回滚恢复实现：
 * 1. 操作日志：详细记录DDL操作的执行过程
 * 2. 逆操作生成：为每个操作生成对应的逆操作
 * 3. 状态保存：保存执行前的系统状态
 * 4. 异常检测：检测执行过程中的异常情况
 * 5. 回滚执行：按逆序执行回滚操作
 * 6. 状态验证：验证回滚后的系统状态正确性
 *
 * 性能优化实现：
 * 1. 操作合并：将多个相关DDL操作合并执行
 * 2. 延迟优化：延迟非关键操作的执行
 * 3. 并行处理：利用多核并行执行DDL操作
 * 4. 资源预留：预留必要的系统资源
 * 5. 执行顺序：优化DDL操作的执行顺序
 * 6. 性能监控：实时监控执行性能指标
 *
 * 错误处理实现：
 * 1. 语法错误：DDL语句语法错误的处理
 * 2. 语义错误：DDL语句语义错误的处理
 * 3. 权限错误：权限不足导致的执行错误
 * 4. 并发错误：并发冲突导致的执行错误
 * 5. 资源错误：资源不足导致的执行错误
 * 6. 系统错误：系统异常导致的执行错误
 *
 * 扩展性设计：
 * - 插件架构：支持第三方DDL处理器插件
 * - 自定义DDL：支持用户自定义的DDL语句类型
 * - 多数据库：支持不同数据库系统的DDL语法
 * - 分布式DDL：支持分布式环境下的DDL操作
 * - AI优化：基于机器学习的DDL优化
 *
 * 调试和诊断：
 * - 执行跟踪：详细记录DDL执行的每一步过程
 * - 性能分析：分析DDL操作的性能瓶颈和优化机会
 * - 错误诊断：提供详细的错误诊断和解决建议
 * - 状态监控：监控系统状态和元数据的变化
 * - 可视化工具：DDL执行过程和结果的可视化展示
 * - 审计日志：完整的DDL操作审计日志记录
  */
 
 /**
  * @file ddl_execution_strategy.h
  * @brief DDL执行策略头文件
  */
 #ifndef SQLCC_EXECUTION_DDL_EXECUTION_STRATEGY_H
 #define SQLCC_EXECUTION_DDL_EXECUTION_STRATEGY_H
 
 #include "src/sql_parser/ast/ast_node.h"
 #include "src/sql_parser/ast/ast_nodes.h"
 #include <memory>
#include <string>

#include "src/execution/execution_strategy.h"
#include "src/core/execution_result.h"

namespace sqlcc {

namespace sql_parser {
class Statement;
class CreateTableStatement;
class DropTableStatement;
class AlterTableStatement;
class CreateIndexStatement;
class DropIndexStatement;
} // namespace sql_parser

class ExecutionContext;

// DDL执行策略 - 处理数据定义语言语句
class DDLExecutionStrategy : public ExecutionStrategy {
public:
    DDLExecutionStrategy();
    ~DDLExecutionStrategy() override = default;

    // 执行DDL语句
    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                           ExecutionContext& context) override;
    bool checkPermission(const sql_parser::Statement& stmt,
                        const ExecutionContext& context) override;
    bool validate(const sql_parser::Statement& stmt,
                 const ExecutionContext& context) override;
    std::string getStrategyName() const override;

private:
    // DDL语句处理
    ExecutionResult executeCreateTable(const sql_parser::CreateTableStatement& stmt,
                                      ExecutionContext& context);
    ExecutionResult executeDropTable(const sql_parser::DropTableStatement& stmt,
                                    ExecutionContext& context);
    ExecutionResult executeAlterTable(const sql_parser::AlterTableStatement& stmt,
                                     ExecutionContext& context);
    ExecutionResult executeCreateIndex(const sql_parser::CreateIndexStatement& stmt,
                                      ExecutionContext& context);
    ExecutionResult executeDropIndex(const sql_parser::DropIndexStatement& stmt,
                                    ExecutionContext& context);

    // 验证方法
    bool validateTableName(const std::string& table_name, const ExecutionContext& context) const;
    bool validateIndexName(const std::string& index_name, const ExecutionContext& context) const;
    bool validateColumnDefinitions(const std::vector<sql_parser::ColumnDefinition>& columns) const;
    bool checkTableExists(const std::string& table_name, const ExecutionContext& context) const;
    bool checkIndexExists(const std::string& index_name, const ExecutionContext& context) const;

    // 权限检查
    bool hasCreateTablePermission(const ExecutionContext& context) const;
    bool hasDropTablePermission(const std::string& table_name, const ExecutionContext& context) const;
    bool hasAlterTablePermission(const std::string& table_name, const ExecutionContext& context) const;
    bool hasCreateIndexPermission(const ExecutionContext& context) const;
    bool hasDropIndexPermission(const std::string& index_name, const ExecutionContext& context) const;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_DDL_EXECUTION_STRATEGY_H
