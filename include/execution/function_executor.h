#ifndef SQLCC_EXECUTION_FUNCTION_EXECUTOR_H
#define SQLCC_EXECUTION_FUNCTION_EXECUTOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

#include "types/domain_manager.h"
#include "sql_parser/function/function_definition.h"

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
                                   const std::vector<Value>& arguments);

    // 执行SQL查询
    Value executeSqlQuery(const std::string& sql,
                         std::shared_ptr<SqlExecutor> executor);
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
