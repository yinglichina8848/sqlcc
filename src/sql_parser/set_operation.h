/**
 * WHY: 为什么需要专门的集合操作系统？
 *
 * 集合操作是SQL查询的核心高级特性，传统数据库系统存在诸多集合操作处理问题：
 * - 语法复杂：集合操作包含UNION、INTERSECT、EXCEPT等操作符，语法规则多样
 * - 语义约束：不同集合操作有不同的语义要求和结果集兼容性检查
 * - 性能瓶颈：集合操作需要复杂的排序、去重和合并算法
 * - 标准兼容：需要完全支持SQL标准的所有集合操作特性
 * - 扩展性差：难以添加新的集合操作类型和优化策略
 *
 * 集合操作系统的核心价值：
 * 1. 查询能力：支持复杂的多集合查询和数据集合操作
 * 2. 性能优化：高效的集合操作算法和内存管理
 * 3. 标准兼容：完全符合SQL标准集合操作规范
 * 4. 语义正确：保证集合操作的数学语义和结果准确性
 * 5. 灵活组合：支持多重集合操作的嵌套和组合
 *
 * 🏗️ 设计模式：组合模式(Composite Pattern)
 *
 * 集合操作作为组合模式的应用：
 * - 递归结构：集合操作可以递归组合形成复杂的查询树
 * - 统一接口：所有集合操作实现统一的接口和行为
 * - 树形结构：集合操作形成二叉树结构，支持多重嵌套
 * - 动态构建：运行时动态构建集合操作的执行计划
 * - 结果合并：统一的集合操作结果合并和处理机制
 *
 * SOLID原则体现：
 * - 单一职责：集合操作类负责单一的集合计算逻辑
 * - 开闭原则：新集合操作通过扩展现有类实现
 * - 里氏替换：集合操作子类可以替换基类使用
 * - 接口隔离：集合操作接口精确定义所需方法
 * - 依赖倒置：高层模块依赖集合操作接口而非实现
 *
 * WHAT: SQL集合操作系统 - 完整的多集合查询框架
 *
 * 核心功能：
 * - 并集操作：UNION和UNION ALL，支持去重和保留重复
 * - 交集操作：INTERSECT，计算两个集合的交集
 * - 差集操作：EXCEPT，计算两个集合的差集
 * - 排序支持：ORDER BY子句对最终结果排序
 * - 限制支持：LIMIT子句限制结果集大小
 *
 * 系统组件：
 * - SetOperation：集合操作表达式的AST节点表示
 * - SetOperationType：集合操作类型的枚举定义
 * - SelectStatement：集合操作的操作数（SELECT语句）
 * - ORDER BY：集合操作结果的排序子句
 * - LIMIT：集合操作结果的限制子句
 *
 * 集合操作类型：
 * - UNION：标准并集操作，去除重复行
 * - UNION ALL：并集操作，保留所有重复行
 * - INTERSECT：交集操作，只保留两个集合共有的行
 * - EXCEPT：差集操作，只保留第一个集合有而第二个集合没有的行
 *
 * 结果集兼容性：
 * - 列数匹配：两个操作数的SELECT语句必须有相同数量的列
 * - 类型兼容：对应列的数据类型必须兼容或可转换
 * - 名称映射：结果集的列名根据第一个操作数确定
 * - NULL处理：正确处理NULL值在集合操作中的语义
 *
 * 接口设计：
 * - 操作创建：根据操作类型创建集合操作对象
 * - 操作数设置：设置左操作数和右操作数
 * - 选项配置：配置ALL标志和排序限制选项
 * - 结果查询：获取操作类型和操作数信息的查询接口
 *
 * HOW: SQL集合操作系统的实现机制
 *
 * 集合操作解析流程：
 * 1. 语法识别：识别集合操作关键字和操作符
 * 2. 操作分类：根据关键字确定集合操作类型
 * 3. 操作数解析：递归解析左操作数和右操作数
 * 4. 选项处理：处理ALL关键字和其他选项
 * 5. 语义验证：验证操作数的兼容性和语法正确性
 * 6. 对象构建：创建集合操作AST节点和相关对象
 *
 * 集合操作执行流程：
 * 1. 左操作执行：执行左操作数的SELECT语句
 * 2. 右操作执行：执行右操作数的SELECT语句
 * 3. 类型转换：确保两个结果集的类型兼容
 * 4. 集合计算：根据操作类型执行相应的集合算法
 * 5. 结果处理：应用ORDER BY和LIMIT子句
 * 6. 结果返回：返回最终的集合操作结果
 *
 * 并集操作实现：
 * 1. UNION：对两个结果集合并并去除重复行
 * 2. UNION ALL：对两个结果集直接合并保留重复
 * 3. 排序去重：使用排序算法实现高效的去重操作
 * 4. 哈希去重：使用哈希表实现快速的重复检测
 * 5. 内存优化：分批处理大结果集避免内存溢出
 *
 * 交集操作实现：
 * 1. INTERSECT：计算两个集合的公共元素
 * 2. 排序算法：使用排序合并算法实现交集计算
 * 3. 哈希算法：使用哈希表实现快速交集查找
 * 4. 位图优化：使用位图索引优化交集操作
 * 5. 索引利用：利用现有索引加速交集计算
 *
 * 差集操作实现：
 * 1. EXCEPT：计算第一个集合减去第二个集合的结果
 * 2. 排序算法：使用排序算法实现差集计算
 * 3. 哈希算法：使用哈希表实现快速差集查找
 * 4. 反向索引：利用索引的反向查找能力
 * 5. 内存管理：优化内存使用处理大集合差集
 *
 * 结果集兼容性检查：
 * 1. 列数验证：确保两个操作数有相同数量的列
 * 2. 类型匹配：检查对应列的数据类型兼容性
 * 3. 精度处理：处理数值类型的精度和范围差异
 * 4. 字符集：验证字符数据的字符集兼容性
 * 5. 时区处理：处理时间戳数据的时区信息
 *
 * 性能优化策略：
 * - 流水线执行：并行执行多个集合操作
 * - 中间结果缓存：缓存中间集合操作结果
 * - 索引优化：利用索引加速集合操作
 - 算法选择：根据数据特征选择最优算法
 * - 内存管理：优化内存使用减少GC压力
 *
 * 内存管理策略：
 * - 流式处理：对大结果集使用流式集合操作
 * - 分批处理：将大集合操作分解为小批次处理
 * - 结果复用：复用中间结果避免重复计算
 * - 垃圾回收：及时清理不再需要的中间结果
 * - 内存监控：监控集合操作的内存使用情况
 *
 * 错误处理机制：
 * - 语法错误：集合操作语法错误的详细诊断
 * - 语义错误：集合操作数不兼容的检测
 * - 类型错误：数据类型不匹配的验证
 * - 资源错误：内存不足等资源错误的处理
 * - 结果错误：集合操作结果异常的检测和报告
 *
 * 扩展性设计：
 * - 插件架构：支持自定义集合操作的动态加载
 * - 配置化：集合操作行为的配置化管理
 * - 多语言支持：不同SQL方言的集合操作语法
 * - 标准化：严格遵循SQL标准的集合操作规范
 * - 向后兼容：保持与现有集合操作系统的兼容性
 *
 * 调试和诊断：
 * - 执行计划：可视化集合操作的执行计划
 * - 性能分析：集合操作的详细性能统计
 * - 中间结果：检查集合操作的中间步骤结果
 * - 兼容性报告：详细的集合兼容性检查报告
 * - 测试工具：集合操作逻辑的单元测试框架
 */

#include "ast/ast_nodes.h"
#ifndef SQLCC_SQL_PARSER_SET_OPERATION_H
#define SQLCC_SQL_PARSER_SET_OPERATION_H

#include "ast/ast_node.h"
#include "ast/node_visitor.h"
#include <memory>
#include <string>
#include <vector>

namespace sql::ast {

// Forward declarations
class SelectStatement;

/**
 * 集合操作类型枚举
 */
enum class SetOperationType {
    UNION,      // UNION操作
    INTERSECT,  // INTERSECT操作
    EXCEPT      // EXCEPT操作
};

/**
 * 集合操作节点类
 * 表示包含集合操作的复合查询语句
 */
class SetOperation : public Statement {
public:
    /**
     * 构造函数
     * @param operationType 集合操作类型
     * @param leftOperand 左操作数（Select语句）
     * @param rightOperand 右操作数（Select语句）
     * @param allFlag 是否包含ALL关键字（默认为false）
     */
    SetOperation(SetOperationType operationType, 
                 std::unique_ptr<SelectStatement> leftOperand,
                 std::unique_ptr<SelectStatement> rightOperand,
                 bool allFlag = false);
    
    ~SetOperation();

    // Getters
    SetOperationType getOperationType() const;
    const std::string& getOperationName() const;
    SelectStatement* getLeftOperand() const;
    SelectStatement* getRightOperand() const;
    bool isAll() const;

    // ORDER BY support
    void setOrderBy(std::vector<std::string> columns, std::vector<bool> ascending);
    const std::vector<std::string>& getOrderByColumns() const;
    const std::vector<bool>& getOrderByAscending() const;
    bool hasOrderBy() const;

    // LIMIT support
    void setLimit(size_t limit);
    size_t getLimit() const;
    bool hasLimit() const;

    // Node interface
    void accept(NodeVisitor& visitor) override;

private:
    SetOperationType operationType_;
    std::unique_ptr<SelectStatement> leftOperand_;
    std::unique_ptr<SelectStatement> rightOperand_;
    bool allFlag_;
    std::string operationName_;
    
    // ORDER BY support
    std::vector<std::string> orderByColumns_;
    std::vector<bool> orderByAscending_;
    
    // LIMIT support
    size_t limit_;
    bool hasLimit_;
};

} // namespace sql::ast

#endif // SQLCC_SQL_PARSER_SET_OPERATION_H
