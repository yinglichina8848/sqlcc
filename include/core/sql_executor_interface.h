#ifndef SQLCC_SQL_EXECUTOR_INTERFACE_H
#define SQLCC_SQL_EXECUTOR_INTERFACE_H

#include <string>
#include <memory>

namespace sqlcc {

/**
 * @brief SQL执行器接口 - 用于打破循环依赖
 *
 * 这个接口定义了SQL执行器的基本功能，
 * 允许其他组件通过接口依赖而不是具体实现。
 */
class SqlExecutorInterface {
public:
    virtual ~SqlExecutorInterface() = default;

    /**
     * @brief 执行SQL语句
     * @param sql SQL语句字符串
     * @return 执行结果消息
     */
    virtual std::string Execute(const std::string& sql) = 0;

    /**
     * @brief 获取最后一次执行的错误信息
     * @return 错误信息
     */
    virtual std::string GetLastError() const = 0;
};

} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_INTERFACE_H
