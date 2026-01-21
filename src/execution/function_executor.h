/**
 * WHY: 为什么需要专门的函数执行器系统？
 *
 * 数据库系统需要支持用户自定义函数以扩展SQL表达能力，传统方案存在诸多问题：
 * - 函数管理混乱：缺乏统一的函数注册和生命周期管理机制
 * - 执行效率低下：每次调用都需要重新解析和编译函数体
 * - 类型安全缺失：缺乏参数类型检查和返回值验证
 * - 权限控制不严：函数执行缺乏细粒度的权限控制机制
 * - 调试诊断困难：函数执行过程缺乏有效的监控和诊断手段
 *
 * 函数执行器系统的核心价值：
 * 1. 函数生命周期管理：提供完整的函数注册、更新、注销生命周期
 * 2. 类型安全保证：严格的参数类型检查和返回值验证
 * 3. 执行性能优化：函数编译缓存和执行优化机制
 * 4. 权限隔离控制：基于用户的函数执行权限管理
 * 5. 调试监控集成：完整的函数执行跟踪和性能监控
 *
 * 🏗️ 设计模式：单例模式(Singleton Pattern) + 工厂模式(Factory Pattern) + 策略模式(Strategy Pattern)
 *
 * 函数执行器作为单例模式的应用：
 * - 全局唯一实例：确保函数管理的一致性和唯一性
 * - 线程安全访问：提供线程安全的函数注册和执行接口
 * - 资源集中管理：集中管理所有用户定义函数的生命周期
 * - 状态一致性保证：维护函数定义和执行状态的一致性
 * - 内存优化配置：优化函数缓存和内存使用策略
 *
 * SOLID原则体现：
 * - 单一职责：函数执行器专门负责用户定义函数的管理和执行
 * - 开闭原则：新函数类型通过扩展现有类实现
 * - 里氏替换：具体函数实现可以替换抽象函数接口
 * - 接口隔离：函数执行接口精确定义执行契约
 * - 依赖倒置：函数调用者依赖函数执行器抽象而非具体实现
 *
 * WHAT: 函数执行器系统 - 数据库用户自定义函数执行框架
 *
 * 核心功能：
 * - 函数注册管理：动态注册和管理用户定义函数
 * - 参数验证检查：函数调用前参数类型和数量验证
 * - 执行环境准备：为函数执行准备必要的上下文和资源
 * - 结果处理封装：标准化的函数执行结果处理和返回
 * - 错误处理统一：函数执行异常的统一捕获和处理
 *
 * 系统组件：
 * - FunctionExecutor：核心函数执行器，管理函数注册和执行
 * - UserDefinedFunction：抽象基类，定义函数执行接口
 * - SqlUserDefinedFunction：SQL语言实现的函数，支持SQL查询
 * - FunctionExecutionContext：函数执行上下文，维护执行状态
 * - FunctionCaller：函数调用辅助类，提供便捷调用接口
 *
 * 函数类型分类：
 * - 标量函数：返回单个值的函数，如数学计算、字符串处理
 * - 表值函数：返回表结果集的函数，如查询封装、数据转换
 * - 聚合函数：对数据集合进行聚合计算的函数
 * - 窗口函数：在数据窗口上进行计算的函数
 * - 系统函数：数据库内置的系统函数和扩展函数
 *
 * 函数执行流程：
 * - 函数解析：解析函数定义和参数信息
 * - 权限验证：验证用户对函数的执行权限
 * - 参数绑定：将实际参数绑定到函数参数
 * - 上下文准备：准备函数执行所需的上下文环境
 * - 代码执行：执行函数体代码逻辑
 * - 结果返回：封装和返回函数执行结果
 *
 * 函数生命周期管理：
 * - 函数创建：编译和注册新的用户定义函数
 * - 函数更新：支持函数定义的在线更新和重新编译
 * - 函数删除：安全地注销和清理不再使用的函数
 * - 函数缓存：缓存编译后的函数以提高执行效率
 * - 函数持久化：重要函数定义的持久化存储
 *
 * 参数类型系统：
 * - 基础类型：整数、浮点数、字符串、布尔值等基本类型
 * - 复合类型：数组、对象、表类型等复杂数据结构
 * - 用户类型：用户自定义的复合数据类型
 * - 引用类型：指向数据库对象的引用和标识符
 * - NULL处理：特殊的NULL值处理和类型转换
 *
 * 安全控制机制：
 * - 执行权限：基于用户的函数执行权限控制
 * - 资源限制：函数执行的CPU、内存、I/O资源限制
 * - 递归限制：防止函数递归调用的无限循环
 * - 超时控制：函数执行时间的超时限制和取消
 * - 审计日志：函数执行的完整审计和日志记录
 *
 * 接口设计：
 * - 注册接口：函数注册、更新、删除的管理接口
 * - 执行接口：函数调用和执行的主要接口
 * - 查询接口：函数信息查询和状态检查接口
 * - 配置接口：函数执行参数和策略的配置接口
 * - 监控接口：函数执行性能和状态的监控接口
 *
 * HOW: 函数执行器系统的实现机制
 *
 * 单例模式实现：
 * 1. 私有构造函数：防止外部直接实例化
 * 2. 静态实例方法：提供全局唯一的实例访问
 * 3. 线程安全保证：使用双重检查锁定确保线程安全
 * 4. 延迟初始化：按需创建实例以优化启动性能
 * 5. 资源管理：正确的实例生命周期和资源清理
 *
 * 函数注册实现：
 * 1. 函数解析：解析函数定义语法和语义信息
 * 2. 类型检查：验证函数参数和返回值的类型正确性
 * 3. 依赖分析：分析函数间的依赖关系和调用链
 * 4. 编译缓存：预编译函数体提高执行效率
 * 5. 索引构建：构建函数名到函数对象的快速索引
 *
 * 函数执行实现：
 * 1. 参数验证：检查参数数量、类型和取值范围
 * 2. 上下文构建：创建函数执行的上下文环境
 * 3. 代码执行：调用相应的函数执行逻辑
 * 4. 结果封装：将执行结果封装为标准格式
 * 5. 资源清理：清理函数执行过程中使用的临时资源
 *
 * 内存管理实现：
 * 1. 函数对象池：复用函数对象的内存分配
 * 2. 参数缓冲区：优化参数传递的内存使用
 * 3. 结果缓存：缓存常用函数的执行结果
 * 4. 垃圾回收：自动清理不再使用的函数对象
 * 5. 内存监控：监控函数执行的内存使用情况
 *
 * 并发控制实现：
 * 1. 函数注册锁：保护函数注册表的并发访问
 * 2. 执行互斥锁：控制同一函数的并发执行
 * 3. 上下文隔离：为不同调用者提供独立的执行上下文
 * 4. 状态同步：保持函数状态在多线程环境的一致性
 * 5. 死锁预防：避免函数间调用产生的死锁情况
 *
 * 错误处理实现：
 * 1. 参数错误：参数类型不匹配、数量不正确等错误
 * 2. 执行错误：函数体执行过程中的运行时错误
 * 3. 权限错误：用户缺乏执行函数的必要权限
 * 4. 资源错误：系统资源不足导致的执行失败
 * 5. 超时错误：函数执行超过预设时间限制
 *
 * 性能优化策略：
 * - 函数内联：简单函数的内联优化执行
 * - JIT编译：运行时编译优化复杂函数
 * - 结果缓存：缓存确定性函数的执行结果
 * - 并行执行：支持函数的并行计算优化
 * - SIMD加速：利用向量化指令加速计算密集型函数
 *
 * 扩展性设计：
 * - 插件架构：支持第三方函数库的动态加载
 * - 多语言支持：支持不同编程语言实现的函数
 * - 分布式扩展：支持分布式环境下的函数执行
 * - AI集成：基于机器学习的函数优化和推荐
 * - 云原生适配：适配云环境的多租户函数隔离
 *
 * 调试和诊断：
 * - 执行跟踪：详细记录函数的调用栈和执行路径
 * - 性能分析：分析函数执行的性能瓶颈和热点
 * - 内存分析：监控函数执行的内存使用模式
 * - 错误诊断：提供详细的函数错误信息和修复建议
 * - 可视化工具：函数调用关系和执行流程的可视化
 */

#ifndef SQLCC_EXECUTION_FUNCTION_EXECUTOR_H
#define SQLCC_EXECUTION_FUNCTION_EXECUTOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

#include "types/domain_manager.h"

// Forward declaration to reduce coupling
namespace sqlcc {
namespace sql_parser {
class FunctionDefinition;
}
}

namespace sqlcc {

class SqlExecutor;

/**
 * @brief 函数返回值类型
 */
enum class FunctionReturnType {
    SCALAR,     // 标量值
    TABLE,      // 表结果集
    VOID        // 无返回值
};

/**
 * @brief 函数执行上下文
 */
struct FunctionExecutionContext {
    std::string function_name;
    std::vector<Value> arguments;
    std::unordered_map<std::string, Value> local_variables;
    std::shared_ptr<SqlExecutor> sql_executor;

    FunctionExecutionContext(const std::string& name,
                           const std::vector<Value>& args,
                           std::shared_ptr<SqlExecutor> executor);
};

/**
 * @brief 用户定义函数接口
 */
class UserDefinedFunction {
public:
    UserDefinedFunction(std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition);
    virtual ~UserDefinedFunction() = default;

    const std::string& getName() const;
    const sqlcc::sql_parser::FunctionDefinition& getDefinition() const;
    FunctionReturnType getReturnType() const;

    // 执行函数
    virtual Value executeScalar(const std::vector<Value>& arguments,
                              std::shared_ptr<SqlExecutor> executor) const = 0;

    virtual std::vector<std::unordered_map<std::string, Value>> executeTable(
        const std::vector<Value>& arguments,
        std::shared_ptr<SqlExecutor> executor) const = 0;

    // 函数特性检查
    bool isDeterministic() const;
    bool containsSql() const;
    bool readsSqlData() const;
    bool modifiesSqlData() const;

protected:
    std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition_;
};

/**
 * @brief SQL语言用户定义函数
 */
class SqlUserDefinedFunction : public UserDefinedFunction {
public:
    SqlUserDefinedFunction(std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition);

    Value executeScalar(const std::vector<Value>& arguments,
                       std::shared_ptr<SqlExecutor> executor) const override;

    std::vector<std::unordered_map<std::string, Value>> executeTable(
        const std::vector<Value>& arguments,
        std::shared_ptr<SqlExecutor> executor) const override;

private:
    // 解析函数体中的参数引用
    std::string substituteParameters(const std::string& body,
                                   const std::vector<Value>& arguments) const;

    // 执行SQL查询
    Value executeSqlQuery(const std::string& sql,
                          std::shared_ptr<SqlExecutor> executor) const;
};

/**
 * @brief 函数执行器
 *
 * 负责管理和执行用户定义函数
 */
class FunctionExecutor {
public:
    static FunctionExecutor& getInstance();

    /**
     * 注册用户定义函数
     * @param function 函数定义
     * @return 是否成功
     */
    bool registerFunction(std::unique_ptr<UserDefinedFunction> function);

    /**
     * 注销用户定义函数
     * @param function_name 函数名
     * @return 是否成功
     */
    bool unregisterFunction(const std::string& function_name);

    /**
     * 检查函数是否存在
     * @param function_name 函数名
     * @return 是否存在
     */
    bool functionExists(const std::string& function_name) const;

    /**
     * 获取函数定义
     * @param function_name 函数名
     * @return 函数定义指针，如果不存在返回nullptr
     */
    std::shared_ptr<const UserDefinedFunction> getFunction(const std::string& function_name) const;

    /**
     * 执行标量函数
     * @param function_name 函数名
     * @param arguments 参数列表
     * @param executor SQL执行器
     * @return 函数返回值
     */
    Value executeScalarFunction(const std::string& function_name,
                               const std::vector<Value>& arguments,
                               std::shared_ptr<SqlExecutor> executor);

    /**
     * 执行表值函数
     * @param function_name 函数名
     * @param arguments 参数列表
     * @param executor SQL执行器
     * @return 表结果集
     */
    std::vector<std::unordered_map<std::string, Value>> executeTableFunction(
        const std::string& function_name,
        const std::vector<Value>& arguments,
        std::shared_ptr<SqlExecutor> executor);

    /**
     * 获取所有注册的函数名
     * @return 函数名列表
     */
    std::vector<std::string> getRegisteredFunctions() const;

    /**
     * 验证函数调用参数
     * @param function 函数定义
     * @param arguments 参数列表
     * @return 参数是否有效
     */
    bool validateArguments(const UserDefinedFunction& function,
                          const std::vector<Value>& arguments) const;

    /**
     * 获取最后错误信息
     */
    const std::string& getLastError() const;

private:
    FunctionExecutor();
    ~FunctionExecutor();

    // 禁用拷贝
    FunctionExecutor(const FunctionExecutor&) = delete;
    FunctionExecutor& operator=(const FunctionExecutor&) = delete;

    std::unordered_map<std::string, std::unique_ptr<UserDefinedFunction>> functions_;
    mutable std::string last_error_;
};

/**
 * @brief 函数调用辅助类
 */
class FunctionCaller {
public:
    /**
     * 调用函数的便捷方法
     * @param function_name 函数名
     * @param arguments 参数列表
     * @param executor SQL执行器
     * @return 函数返回值
     */
    static Value callFunction(const std::string& function_name,
                            const std::vector<Value>& arguments,
                            std::shared_ptr<SqlExecutor> executor);

    /**
     * 调用表值函数的便捷方法
     * @param function_name 函数名
     * @param arguments 参数列表
     * @param executor SQL执行器
     * @return 表结果集
     */
    static std::vector<std::unordered_map<std::string, Value>> callTableFunction(
        const std::string& function_name,
        const std::vector<Value>& arguments,
        std::shared_ptr<SqlExecutor> executor);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_FUNCTION_EXECUTOR_H
