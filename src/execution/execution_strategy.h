/**
 * WHY: 为什么需要专门的执行策略抽象基类？
 *
 * SQL语句执行是数据库系统的核心功能，但传统方案存在诸多技术挑战：
 * - 执行逻辑复杂多样：不同类型的SQL语句需要不同的执行逻辑和优化策略
 * - 权限控制不统一：各种SQL语句的权限检查逻辑分散且不一致
 * - 错误处理不规范：不同执行器对错误的处理方式各不相同
 * - 扩展性设计不足：难以添加新的SQL语句类型和执行策略
 * - 代码复用度低：相似的执行逻辑在不同地方重复实现
 * - 接口不统一：执行器之间的接口和契约定义不规范
 * - 测试维护困难：不同执行器的测试方式和标准不统一
 *
 * 执行策略抽象基类的核心价值：
 * 1. 执行接口标准化：定义统一的SQL语句执行接口和契约
 * 2. 权限控制统一化：提供标准化的权限检查和验证机制
 * 3. 错误处理规范化：建立统一的错误处理和报告机制
 * 4. 策略模式支撑：为策略模式提供基础架构和扩展机制
 * 5. 代码复用最大化：提供通用的执行辅助方法和工具函数
 * 6. 扩展性保障：支持新执行策略的无缝集成和扩展
 * 7. 测试标准化：提供统一的测试接口和验证标准
 *
 * 🏗️ 设计模式：模板方法模式(Template Method Pattern) + 策略模式(Strategy Pattern) + 抽象工厂模式(Abstract Factory Pattern)
 *
 * 执行策略作为模板方法模式的应用：
 * - 执行流程模板化：定义SQL语句执行的标准流程和步骤
 * - 步骤定制化：允许子类定制具体的执行步骤和逻辑
 * - 流程一致性保障：确保所有执行器遵循相同的执行流程
 * - 扩展点明确化：明确定义可扩展和可定制的执行点
 * - 错误处理标准化：统一的错误处理和恢复机制
 *
 * SOLID原则体现：
 * - 单一职责：执行策略专门负责SQL语句执行的接口定义和管理
 * - 开闭原则：新执行策略通过继承实现，不修改现有代码
 * - 里氏替换：所有执行策略都可以互相替换使用
 * - 接口隔离：执行策略接口精确定义执行契约
 * - 依赖倒置：执行策略依赖抽象的上下文和结果接口
 *
 * WHAT: 执行策略抽象基类系统 - SQL语句执行统一框架
 *
 * 核心功能：
 * - SQL语句执行接口：定义标准的SQL语句执行接口和行为
 * - 权限验证机制：统一的权限检查和访问控制机制
 * - 语句验证框架：标准的SQL语句验证和语法检查
 * - 执行结果封装：统一的执行结果封装和返回机制
 * - 错误处理规范：标准化的错误处理和异常管理
 * - 策略命名管理：执行策略的标识和命名管理
 * - 辅助工具方法：通用的执行辅助方法和工具函数
 *
 * 系统组件：
 * - ExecutionStrategy：抽象基类，定义执行策略的标准接口
 * - DDLExecutionStrategy：DDL执行策略，处理数据定义语句
 * - DMLExecutionStrategy：DML执行策略，处理数据操作语句
 * - ExecutionContext：执行上下文，提供执行所需的上下文信息
 * - ExecutionResult：执行结果，封装执行的成功与失败信息
 * - PermissionValidator：权限验证器，验证执行权限
 * - StatementValidator：语句验证器，验证SQL语句语法
 *
 * 执行流程：
 * - 语句接收：接收待执行的SQL语句和执行上下文
 * - 权限预检查：在执行前验证用户的执行权限
 * - 语句验证：在执行前验证SQL语句的语法和语义正确性
 * - 执行准备：准备执行所需的资源和环境
 * - 语句执行：执行具体的SQL语句处理逻辑
 * - 结果封装：将执行结果封装为标准格式
 * - 资源清理：清理执行过程中使用的临时资源
 * - 结果返回：返回执行结果给调用者
 *
 * 权限验证机制：
 * - 用户身份识别：识别当前执行用户的身份信息
 * - 权限规则评估：根据权限规则评估用户执行权限
 * - 权限缓存利用：利用权限缓存提高验证性能
 * - 权限审计记录：记录权限验证的结果和过程
 * - 权限策略配置：灵活配置的权限验证策略
 * - 权限异常处理：权限验证失败的异常处理
 *
 * 语句验证框架：
 * - 语法验证：验证SQL语句的语法正确性
 * - 语义验证：验证SQL语句的语义合理性
 * - 逻辑验证：验证SQL语句的逻辑一致性
 * - 依赖验证：验证SQL语句的依赖关系正确性
 * - 约束验证：验证SQL语句满足数据库约束
 * - 类型验证：验证SQL语句的数据类型正确性
 *
 * 错误处理规范：
 * - 异常分类：将异常分类为不同类型和级别
 * - 错误信息标准化：提供标准化的错误信息格式
 * - 错误恢复策略：定义不同错误的恢复策略
 * - 错误日志记录：详细记录错误发生的情况和原因
 * - 错误统计分析：统计和分析系统错误模式
 * - 错误预警机制：主动发现和预警潜在错误
 *
 * 策略命名管理：
 * - 策略唯一标识：为每个执行策略提供唯一的标识符
 * - 策略版本控制：执行策略的版本管理和兼容性
 * - 策略配置管理：执行策略的参数配置和管理
 * - 策略性能监控：执行策略的性能监控和统计
 * - 策略动态加载：支持执行策略的动态加载和卸载
 * - 策略依赖管理：管理执行策略之间的依赖关系
 *
 * 辅助工具方法：
 * - 权限检查工具：通用的权限检查辅助方法
 * - 结果创建工具：标准化的执行结果创建方法
 * - 错误处理工具：统一的错误处理和报告工具
 * - 资源管理工具：执行资源的分配和管理工具
 * - 日志记录工具：执行过程的日志记录工具
 * - 性能监控工具：执行性能的监控和统计工具
 *
 * 接口设计：
 * - 执行接口：主要的SQL语句执行接口
 * - 验证接口：SQL语句验证和检查接口
 * - 权限接口：执行权限验证接口
 * - 配置接口：执行策略配置接口
 * - 监控接口：执行监控和统计接口
 * - 扩展接口：执行策略扩展接口
 *
 * HOW: 执行策略抽象基类系统的实现机制
 *
 * 模板方法模式实现：
 * 1. 抽象基类定义：定义执行策略的抽象接口和通用行为
 * 2. 模板方法实现：定义execute方法的执行流程模板
 * 3. 钩子方法定义：定义可被子类重写的钩子方法
 * 4. 抽象方法声明：声明必须由子类实现的抽象方法
 * 5. 默认实现提供：为可选方法提供默认实现
 * 6. 流程控制逻辑：控制整个执行流程的顺序和逻辑
 *
 * 策略模式支撑：
 * 1. 策略接口定义：定义策略模式的标准接口
 * 2. 策略注册机制：注册和管理不同的执行策略
 * 3. 策略选择逻辑：根据条件选择合适的执行策略
 * 4. 策略切换机制：运行时动态切换执行策略
 * 5. 策略生命周期：管理策略的创建、初始化和销毁
 * 6. 策略配置管理：配置和管理策略的参数和行为
 *
 * 权限验证实现：
 * 1. 权限规则定义：定义权限验证的规则和策略
 * 2. 权限检查逻辑：实现具体的权限检查算法
 * 3. 权限缓存机制：实现权限检查结果的缓存
 * 4. 权限审计系统：记录权限检查的审计日志
 * 5. 权限策略引擎：灵活的权限策略配置引擎
 * 6. 权限异常处理：权限验证失败的异常处理机制
 *
 * 语句验证实现：
 * 1. 验证器接口：定义语句验证的标准接口
 * 2. 语法验证器：实现SQL语句的语法验证
 * 3. 语义验证器：实现SQL语句的语义验证
 * 4. 逻辑验证器：实现SQL语句的逻辑验证
 * 5. 依赖验证器：实现SQL语句的依赖验证
 * 6. 类型验证器：实现SQL语句的类型验证
 *
 * 结果封装实现：
 * 1. 结果对象设计：设计执行结果的标准数据结构
 * 2. 成功结果创建：创建成功执行的结果对象
 * 3. 失败结果创建：创建失败执行的结果对象
 * 4. 结果数据填充：填充结果对象的数据内容
 * 5. 结果序列化：将结果对象序列化为传输格式
 * 6. 结果清理管理：管理结果对象的生命周期
 *
 * 错误处理实现：
 * 1. 异常类型定义：定义不同类型的执行异常
 * 2. 异常捕获机制：捕获执行过程中的各种异常
 * 3. 异常转换逻辑：将底层异常转换为标准异常
 * 4. 异常处理策略：定义不同异常的处理策略
 * 5. 异常日志记录：记录异常的详细信息
 * 6. 异常恢复机制：异常发生后的恢复和清理机制
 *
 * 策略命名实现：
 * 1. 命名规范定义：定义执行策略的命名规范
 * 2. 唯一标识生成：生成策略的唯一标识符
 * 3. 版本信息管理：管理策略的版本信息
 * 4. 配置信息关联：关联策略的配置信息
 * 5. 性能指标关联：关联策略的性能指标
 * 6. 依赖关系描述：描述策略间的依赖关系
 *
 * 辅助工具实现：
 * 1. 工具类设计：设计通用的执行辅助工具类
 * 2. 权限检查工具：实现通用的权限检查方法
 * 3. 结果创建工具：实现标准化的结果创建方法
 * 4. 错误处理工具：实现统一的错误处理方法
 * 5. 资源管理工具：实现资源的分配和管理方法
 * 6. 监控统计工具：实现性能监控和统计方法
 *
 * 扩展性设计：
 * - 插件架构：支持第三方执行策略的动态加载
 * - 自定义策略：支持用户自定义的执行策略
 * - 多语言支持：支持多种编程语言的执行策略
 * - 分布式执行：支持分布式环境下的执行策略
 * - AI优化：基于机器学习的执行策略优化
 *
 * 调试和诊断：
 * - 执行跟踪：详细记录执行策略的执行过程
 * - 性能分析：分析执行策略的性能瓶颈和优化机会
 * - 错误诊断：提供详细的错误诊断和解决建议
 * - 状态监控：监控执行策略的运行状态和健康度
 * - 日志分析：分析执行日志发现问题和改进点
 * - 可视化工具：执行过程和结果的可视化展示
 */

/**
 * @file execution_strategy.h
 * @brief 执行策略基类头文件
 */
#ifndef SQLCC_EXECUTION_EXECUTION_STRATEGY_H
#define SQLCC_EXECUTION_EXECUTION_STRATEGY_H

#include "src/sql_parser/ast/ast_nodes.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "src/core/execution_result.h"
#include "src/storage_engine/table_storage.h"  // 包含TableMetadata定义

namespace sqlcc {

namespace sql_parser {
class Statement;
class WhereClause;
} // namespace sql_parser

class ExecutionContext;
class TableMetadata;
struct TableColumn;

// 执行策略基类 - 定义SQL语句执行的策略模式
class ExecutionStrategy {
public:
    virtual ~ExecutionStrategy() = default;
    
    // 执行SQL语句
    virtual ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                                   ExecutionContext& context) = 0;
    // 权限检查
    virtual bool checkPermission(const sql_parser::Statement& stmt,
                                const ExecutionContext& context) {
        // 默认实现
        return true;
    }
    // 语句验证
    virtual bool validate(const sql_parser::Statement& stmt,
                         const ExecutionContext& context) {
        // 默认实现
        return true;
    }
    
    // 获取策略名称
    virtual std::string getStrategyName() const = 0;

protected:
    // 辅助方法
    bool hasRequiredPermissions(const ExecutionContext& context,
                               const std::vector<std::string>& required_permissions) const;
    
    ExecutionResult createErrorResult(const std::string& error_message) const;
    
    ExecutionResult createSuccessResult(const std::string& message = "") const;
    
    // 验证数据库上下文
    bool validateDatabaseContext(const ExecutionContext &context);

    // 验证表是否存在
    bool validateTableExists(const std::string &table_name,
                             const ExecutionContext &context);

    // 更新执行统计信息
    void updateExecutionStats(ExecutionContext &context, size_t records_affected);

    // 生成默认权限检查结果
    bool defaultPermissionCheck(const ExecutionContext &context);

    // 匹配WHERE子句
    bool matchesWhereClause(const std::vector<std::string> &record,
                            const sql_parser::WhereClause &where_clause,
                            std::shared_ptr<TableMetadata> metadata);

    std::string getColumnValue(const std::vector<std::string> &record,
                               const std::string &column_name,
                               std::shared_ptr<TableMetadata> metadata);

    bool compareValues(const std::string &left, const std::string &right,
                       const std::string &op);

    // 约束验证方法
    bool validateColumnConstraints(const std::vector<std::string> &record,
                                   std::shared_ptr<TableMetadata> metadata,
                                   const std::string &table_name);

    bool checkPrimaryKeyConstraints(const std::vector<std::string> &record,
                                    std::shared_ptr<TableMetadata> metadata,
                                    const std::string &table_name);

    bool checkUniqueKeyConstraints(const std::vector<std::string> &record,
                                   std::shared_ptr<TableMetadata> metadata,
                                   const std::string &table_name);

    // 索引维护方法
    void maintainIndexesOnInsert(const std::vector<std::string> &record,
                                 const std::string &table_name, int32_t page_id,
                                 size_t offset, ExecutionContext &context);

    void maintainIndexesOnUpdate(const std::vector<std::string> &old_record,
                                 const std::vector<std::string> &new_record,
                                 const std::string &table_name, int32_t page_id,
                                 size_t offset, ExecutionContext &context);

    void maintainIndexesOnDelete(const std::vector<std::string> &record,
                                 const std::string &table_name, int32_t page_id,
                                 size_t offset, ExecutionContext &context);

    // 权限检查辅助方法
    bool checkCreatePermission(const sql_parser::CreateStatement& stmt,
                               const ExecutionContext& context);
    bool checkSelectPermission(const sql_parser::SelectStatement& stmt,
                               const ExecutionContext& context);
    bool checkInsertPermission(const sql_parser::InsertStatement& stmt,
                               const ExecutionContext& context);
    bool checkUpdatePermission(const sql_parser::UpdateStatement& stmt,
                               const ExecutionContext& context);
    bool checkDeletePermission(const sql_parser::DeleteStatement& stmt,
                               const ExecutionContext& context);
    bool checkDropPermission(const sql_parser::DropStatement& stmt,
                             const ExecutionContext& context);
    bool checkAlterPermission(const sql_parser::AlterStatement& stmt,
                              const ExecutionContext& context);
    bool checkUsePermission(const sql_parser::UseStatement& stmt,
                            const ExecutionContext& context);
    bool checkCreateIndexPermission(const sql_parser::CreateIndexStatement& stmt,
                                    const ExecutionContext& context);
    bool checkDropIndexPermission(const sql_parser::DropIndexStatement& stmt,
                                  const ExecutionContext& context);
    bool checkCreateUserPermission(const sql_parser::CreateUserStatement& stmt,
                                   const ExecutionContext& context);
    bool checkDropUserPermission(const sql_parser::DropUserStatement& stmt,
                                 const ExecutionContext& context);
    bool checkGrantPermission(const sql_parser::GrantStatement& stmt,
                              const ExecutionContext& context);
    bool checkRevokePermission(const sql_parser::RevokeStatement& stmt,
                               const ExecutionContext& context);
    bool checkShowPermission(const sql_parser::ShowStatement& stmt,
                             const ExecutionContext& context);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_EXECUTION_STRATEGY_H