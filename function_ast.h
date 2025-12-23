#ifndef SQLCC_FUNCTION_AST_H_H
#define SQLCC_FUNCTION_AST_H_H

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * @brief FunctionAst 类声明
 *
 * 这是一个自动生成的头文件模板。
 * 请根据实际需求完善类定义。
 */
class FunctionAst {
public:
    // 构造函数
    FunctionAst();
    explicit FunctionAst(const std::string& name);

    // 析构函数
    ~FunctionAst();

    // 禁用拷贝
    FunctionAst(const FunctionAst&) = delete;
    FunctionAst& operator=(const FunctionAst&) = delete;

    // 允许移动
    FunctionAst(FunctionAst&&) noexcept = default;
    FunctionAst& operator=(FunctionAst&&) noexcept = default;

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

#endif // SQLCC_FUNCTION_AST_H_H
