#pragma once

#include <string>

namespace sqlcc {
namespace core {

/**
 * @brief SQL执行器接口
 *
 * 为存储过程模块提供SQL执行能力的接口抽象
 */
class SqlExecutorInterface {
public:
    virtual ~SqlExecutorInterface() = default;

    /**
     * 执行SQL语句
     * @param sql 要执行的SQL语句
     * @return 执行结果
     */
    virtual std::string Execute(const std::string& sql) = 0;

    /**
     * 获取最后一次错误信息
     * @return 错误信息
     */
    virtual std::string GetLastError() const = 0;

    /**
     * 检查是否发生错误
     * @return true表示有错误，false表示无错误
     */
    virtual bool HasError() const = 0;

    /**
     * 清除错误状态
     */
    virtual void ClearError() = 0;
};

} // namespace core
} // namespace sqlcc
