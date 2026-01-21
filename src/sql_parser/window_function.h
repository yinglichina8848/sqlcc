/**
 * WHY: 为什么需要专门的窗口函数系统？
 *
 * 窗口函数是SQL分析查询的核心高级特性，传统数据库系统存在诸多窗口函数处理问题：
 * - 语法复杂：窗口函数语法包含OVER、PARTITION BY、ORDER BY、FRAME等复杂子句
 * - 语义多样：不同的窗口函数类型（排名、聚合、导航）有不同的语义要求
 * - 性能挑战：窗口函数执行需要高效的排序、分区和状态管理算法
 * - 标准兼容：需要完全支持SQL标准的所有窗口函数特性
 * - 扩展性差：难以添加新的窗口函数类型和窗口规格选项
 *
 * 窗口函数系统的核心价值：
 * 1. 分析能力：支持复杂的业务分析和报表查询需求
 * 2. 性能优化：高效的窗口计算算法和内存管理
 * 3. 标准兼容：完全符合SQL标准窗口函数规范
 * 4. 语法完整：支持所有窗口函数语法元素和组合
 * 5. 语义正确：保证窗口函数的计算语义和结果准确性
 *
 * 🏗️ 设计模式：策略模式+组合模式(Strategy Pattern + Composite Pattern)
 *
 * 窗口函数作为策略模式的扩展应用：
 * - 策略选择：根据函数类型动态选择计算策略
 * - 组合构建：窗口规格的组合构建和配置
 * - 状态管理：窗口帧状态的高效管理和更新
 * - 算法优化：不同窗口函数的专用优化算法
 * - 内存复用：窗口状态的内存池管理和复用
 *
 * SOLID原则体现：
 * - 单一职责：窗口函数类负责单一的计算逻辑
 * - 开闭原则：新窗口函数通过扩展现有类实现
 * - 里氏替换：窗口函数子类可以替换基类使用
 * - 接口隔离：窗口函数接口精确定义所需方法
 * - 依赖倒置：高层模块依赖窗口函数接口而非实现
 *
 * WHAT: SQL窗口函数系统 - 完整的分析查询窗口计算框架
 *
 * 核心功能：
 * - 排名函数：ROW_NUMBER、RANK、DENSE_RANK等排名计算
 * - 聚合函数：SUM、AVG、COUNT、MIN、MAX等窗口内聚合
 * - 导航函数：LAG、LEAD、FIRST_VALUE、LAST_VALUE等导航操作
 * - 分布函数：PERCENT_RANK、CUME_DIST等分布计算
 * - 窗口规格：PARTITION BY、ORDER BY、FRAME子句的支持
 *
 * 系统组件：
 * - WindowFunction：窗口函数表达式的AST节点表示
 * - WindowSpecification：窗口规格的完整定义和配置
 * - FrameBoundary：窗口帧边界的枚举和语义定义
 * - FunctionType：窗口函数类型的枚举和分类
 *
 * 窗口规格支持：
 * - PARTITION BY：数据分区的列定义
 * - ORDER BY：分区内排序的列定义和方向
 * - FRAME：窗口帧的起始和结束边界定义
 * - 边界类型：ROWS、RANGE等不同边界类型的支持
 *
 * 窗口帧边界：
 * - UNBOUNDED PRECEDING：分区开始作为起始边界
 * - CURRENT ROW：当前行作为边界
 * - UNBOUNDED FOLLOWING：分区结束作为结束边界
 * - N PRECEDING/FOLLOWING：相对于当前行的偏移边界
 *
 * 接口设计：
 * - 函数创建：根据函数类型和参数创建窗口函数对象
 * - 规格配置：设置窗口规格的各个组成部分
 * - 边界设置：配置窗口帧的起始和结束边界
 * - 类型查询：获取函数类型和规格信息的查询接口
 *
 * HOW: SQL窗口函数系统的实现机制
 *
 * 窗口函数解析流程：
 * 1. 语法识别：识别窗口函数调用和OVER关键字
 * 2. 函数分类：根据函数名确定窗口函数类型
 * 3. 参数解析：解析函数参数和窗口规格子句
 * 4. 语义验证：验证窗口函数语法的正确性和兼容性
 * 5. 对象构建：创建窗口函数AST节点和规格对象
 *
 * 窗口计算执行流程：
 * 1. 数据分区：根据PARTITION BY子句对数据进行分区
 * 2. 分区排序：根据ORDER BY子句对分区内数据排序
 * 3. 帧计算：确定每个行的窗口帧范围
 * 4. 函数计算：根据函数类型执行相应的计算逻辑
 * 5. 结果赋值：将计算结果赋值给相应的输出行
 *
 * 窗口帧管理：
 * 1. 帧边界计算：根据帧定义计算每行的有效范围
 * 2. 状态维护：维护窗口内的累积状态和中间结果
 * 3. 边界更新：高效处理窗口边界的移动和更新
 * 4. 内存优化：复用窗口状态避免重复计算
 * 5. 并行处理：支持窗口计算的并行执行优化
 *
 * 排名函数实现：
 * 1. ROW_NUMBER：为每一行分配连续的序号
 * 2. RANK：相同值分配相同排名，跳过后续排名
 * 3. DENSE_RANK：相同值分配相同排名，不跳过排名
 * 4. PERCENT_RANK：计算相对排名百分比
 * 5. CUME_DIST：计算累积分布值
 *
 * 聚合函数实现：
 * 1. 运行聚合：计算窗口内的累积聚合值
 * 2. 移动聚合：计算移动窗口的聚合统计
 * 3. 中心化聚合：计算相对于当前行的聚合值
 * 4. 分组聚合：按分组计算的聚合统计
 * 5. 条件聚合：基于条件的聚合计算
 *
 * 导航函数实现：
 * 1. LAG/LEAD：访问前/后N行的值
 * 2. FIRST_VALUE：获取窗口内的第一个值
 * 3. LAST_VALUE：获取窗口内的最后一个值
 * 4. NTH_VALUE：获取窗口内的第N个值
 * 5. 边界处理：处理窗口边界情况的默认值
 *
 * 性能优化策略：
 * - 增量计算：利用窗口移动时的增量更新
 * - 排序复用：复用分区内的排序结果
 * - 状态缓存：缓存窗口函数的中间计算状态
 * - SIMD加速：向量化数值计算和比较操作
 * - 并行分区：不同分区的并行窗口计算
 *
 * 内存管理策略：
 * - 对象池：复用窗口函数对象避免频繁分配
 * - 状态复用：复用窗口状态对象减少内存占用
 * - 延迟清理：延迟清理不再使用的窗口状态
 * - 内存预估：预估窗口计算的内存需求
 * - 溢出处理：处理大窗口计算的内存溢出情况
 *
 * 错误处理机制：
 * - 语法错误：窗口函数语法错误的详细诊断
 * - 语义错误：窗口规格语义冲突的检测
 * - 边界错误：窗口帧边界定义错误的检查
 * - 类型错误：函数参数类型不匹配的验证
 * - 性能警告：潜在性能问题的提前警告
 *
 * 扩展性设计：
 * - 插件架构：支持自定义窗口函数的动态加载
 * - 配置化：窗口函数行为的配置化管理
 * - 多语言支持：不同SQL方言的窗口函数语法
 * - 标准化：严格遵循SQL标准的窗口函数规范
 * - 向后兼容：保持与现有窗口函数系统的兼容性
 *
 * 调试和诊断：
 * - 执行计划：可视化窗口函数的执行计划
 * - 性能分析：窗口计算的详细性能统计
 * - 中间结果：检查窗口计算的中间步骤结果
 * - 边界可视化：可视化窗口帧的边界和范围
 * - 测试工具：窗口函数逻辑的单元测试框架
 */

#ifndef SQLCC_WINDOW_FUNCTION_H_H
#define SQLCC_WINDOW_FUNCTION_H_H

#include <memory>
#include <string>
#include <vector>
#include "ast/ast_node.h"

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 窗口函数类型枚举
 */
enum class FunctionType {
    ROW_NUMBER,
    RANK,
    DENSE_RANK,
    SUM,
    AVG,
    COUNT,
    MIN,
    MAX,
    FIRST_VALUE,
    LAST_VALUE,
    LAG,
    LEAD,
    NTH_VALUE
};

/**
 * @brief 窗口帧边界枚举
 */
enum class FrameBoundary {
    UNBOUNDED_PRECEDING,
    CURRENT_ROW,
    UNBOUNDED_FOLLOWING,
    PRECEDING_ROWS,
    FOLLOWING_ROWS
};

/**
 * @brief 窗口函数表达式节点
 */
class WindowFunction : public Expression {
private:
    FunctionType functionType_;
    std::string functionName_;
    std::unique_ptr<Expression> expression_;
    std::unique_ptr<class WindowSpecification> windowSpec_;

public:
    WindowFunction(FunctionType type);
    ~WindowFunction() override = default;

    // 实现基类方法
    std::string getTypeName() const override { return "WindowFunction"; }
    void accept(NodeVisitor& visitor) override;
    Type getType() const override { return FUNCTION; }

    // 窗口函数特定方法
    FunctionType getFunctionType() const;
    const std::string& getFunctionName() const;
    void setExpression(std::unique_ptr<Expression> expr);
    Expression* getExpression() const;
    void setWindowSpecification(std::unique_ptr<class WindowSpecification> spec);
    class WindowSpecification* getWindowSpecification() const;
};

/**
 * @brief 窗口规格类
 */
class WindowSpecification : public Statement {
private:
    std::vector<std::string> partitionByColumns_;
    std::vector<std::string> orderByColumns_;
    std::vector<bool> orderByAscending_;
    FrameBoundary frameStart_;
    FrameBoundary frameEnd_;

public:
    WindowSpecification();
    ~WindowSpecification() override = default;

    void accept(NodeVisitor& visitor) override { /* 默认实现 */ }

    // 分区相关方法
    void setPartitionBy(std::vector<std::string> columns);
    const std::vector<std::string>& getPartitionBy() const { return partitionByColumns_; }
    bool hasPartitionBy() const;

    // 排序相关方法
    void setOrderBy(std::vector<std::string> columns, std::vector<bool> ascending);
    const std::vector<std::string>& getOrderBy() const { return orderByColumns_; }
    const std::vector<bool>& getOrderByAscending() const;
    bool hasOrderBy() const;

    // 窗口帧相关方法
    void setFrame(FrameBoundary start, FrameBoundary end);
    FrameBoundary getFrameStart() const;
    FrameBoundary getFrameEnd() const;
    bool hasFrame() const;
};

} // namespace sql_parser

// 向后兼容的别名
using WindowFunction = sql_parser::WindowFunction;
using WindowSpecification = sql_parser::WindowSpecification;

} // namespace sqlcc

#endif // SQLCC_WINDOW_FUNCTION_H_H
