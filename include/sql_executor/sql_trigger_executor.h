#ifndef SQLCC_SQL_TRIGGER_EXECUTOR_H_H
#define SQLCC_SQL_TRIGGER_EXECUTOR_H_H

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * @brief SqlTriggerExecutor 类声明
 *
 * 这是一个自动生成的头文件模板。
 * 请根据实际需求完善类定义。
 */
class SqlTriggerExecutor {
public:
    // 构造函数
    SqlTriggerExecutor();
    explicit SqlTriggerExecutor(const std::string& name);

    // 析构函数
    ~SqlTriggerExecutor();

    // 禁用拷贝
    SqlTriggerExecutor(const SqlTriggerExecutor&) = delete;
    SqlTriggerExecutor& operator=(const SqlTriggerExecutor&) = delete;

    // 允许移动
    SqlTriggerExecutor(SqlTriggerExecutor&&) noexcept = default;
    SqlTriggerExecutor& operator=(SqlTriggerExecutor&&) noexcept = default;

    // 公共方法
    void initialize();
    void shutdown();

    // Getter/Setter
    const std::string& get_name() const;
    void set_name(const std::string& name);

private:
    std::string name_;
    bool initialized_;
};

} // namespace sqlcc

#endif // SQLCC_SQL_TRIGGER_EXECUTOR_H_H
