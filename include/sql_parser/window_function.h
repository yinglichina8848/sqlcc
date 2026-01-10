#ifndef SQLCC_WINDOW_FUNCTION_H_H
#define SQLCC_WINDOW_FUNCTION_H_H

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * @brief WindowFunction 类声明
 *
 * 这是一个自动生成的头文件模板。
 * 请根据实际需求完善类定义。
 */
class WindowFunction {
public:
    // 构造函数
    WindowFunction();
    explicit WindowFunction(const std::string& name);

    // 析构函数
    ~WindowFunction();

    // 禁用拷贝
    WindowFunction(const WindowFunction&) = delete;
    WindowFunction& operator=(const WindowFunction&) = delete;

    // 允许移动
    WindowFunction(WindowFunction&&) noexcept = default;
    WindowFunction& operator=(WindowFunction&&) noexcept = default;

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

#endif // SQLCC_WINDOW_FUNCTION_H_H
