#ifndef SQLCC_TRIGGER_MANAGER_H_H
#define SQLCC_TRIGGER_MANAGER_H_H

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * @brief TriggerManager 类声明
 *
 * 这是一个自动生成的头文件模板。
 * 请根据实际需求完善类定义。
 */
class TriggerManager {
public:
    // 构造函数
    TriggerManager();
    explicit TriggerManager(const std::string& name);

    // 析构函数
    ~TriggerManager();

    // 禁用拷贝
    TriggerManager(const TriggerManager&) = delete;
    TriggerManager& operator=(const TriggerManager&) = delete;

    // 允许移动
    TriggerManager(TriggerManager&&) noexcept = default;
    TriggerManager& operator=(TriggerManager&&) noexcept = default;

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

#endif // SQLCC_TRIGGER_MANAGER_H_H
