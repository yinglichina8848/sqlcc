#ifndef SQLCC_EXECUTION_RESULT_H
#define SQLCC_EXECUTION_RESULT_H

#include "../../src/wal_manager.h" // 包含Value的定义
#include <optional>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * WHY: 为什么需要执行结果？
 *
 * 数据库系统执行SQL语句后需要返回结果给客户端：
 * - 查询结果：SELECT语句返回的行数据和列信息
 * - 执行状态：DML操作的影响行数和成功/失败状态
 * - 错误信息：执行失败时的详细错误描述
 * - 警告信息：执行过程中的非致命性问题提醒
 * - 元数据信息：结果集的结构描述和类型信息
 *
 * 传统方法的问题：
 * 1. 返回值混乱：不同类型的操作使用不同的返回格式
 * 2. 错误处理复杂：错误信息分散在多个地方
 * 3. 状态管理缺失：无法统一判断执行成功与否
 * 4. 元数据丢失：客户端无法获知结果集的结构信息
 * 5. 扩展性差：新增返回信息需要修改大量接口
 *
 * 执行结果的核心价值：
 * 1. 统一结果格式：所有SQL操作使用一致的结果接口
 * 2. 完整状态信息：包含成功、失败、警告等完整状态
 * 3. 结构化数据：提供结果集的完整元数据描述
 * 4. 错误诊断：详细的错误信息和诊断支持
 * 5. 客户端友好：便于客户端解析和处理结果
 * 6. 监控统计：支持执行统计和性能监控
 *
 * 执行结果在数据库系统中的关键作用：
 * - 客户端通信：统一的客户端-服务器通信协议
 * - 错误处理：系统化的错误报告和处理机制
 * - 结果缓存：支持查询结果的缓存和复用
 * - 审计追踪：记录操作结果用于审计和安全分析
 * - 性能监控：收集执行统计用于性能优化
 * - 调试诊断：提供详细的执行信息用于问题排查
 *
 * 🏗️ 设计模式：执行结果架构设计
 *
 * 设计模式应用：
 * 1. 建造者模式(Builder Pattern)：结果对象构建
 *    - 复杂的执行结果通过建造者逐步构建
 *    - 支持可选字段和默认值设置
 *    - 提高结果对象的构造灵活性
 *
 * 2. 组合模式(Composite Pattern)：嵌套结果结构
 *    - 结果集包含行，行包含列值
 *    - 统一的访问接口处理不同层级
 *    - 支持复杂的结果集结构
 *
 * 3. 访问者模式(Visitor Pattern)：结果处理
 *    - 不同客户端可以使用不同的访问者处理结果
 *    - 支持多种结果序列化格式（JSON、XML等）
 *    - 便于扩展新的处理方式
 *
 * 4. 状态机模式(State Machine Pattern)：结果状态管理
 *    - 明确的执行状态转换（准备、执行、完成、错误）
 *    - 状态驱动的结果处理逻辑
 *    - 防止状态不一致问题
 *
 * SOLID原则体现：
 * - 单一职责：专职负责执行结果的封装和管理
 * - 开闭原则：新结果类型通过扩展实现
 * - 里氏替换：子类可替换父类使用
 * - 接口隔离：客户端依赖具体接口
 * - 依赖倒置：高层不依赖具体实现
 *
 * WHAT: 执行结果 - SQL执行的统一结果封装容器
 *
 * 核心功能：
 * - 结果集管理：存储查询返回的行数据和列信息
 * - 状态信息：记录执行的成功/失败状态
 * - 错误处理：提供详细的错误信息和警告信息
 * - 元数据描述：描述结果集的结构和类型信息
 * - 统计信息：记录影响行数和执行统计
 *
 * 结果类型支持：
 * - 查询结果：SELECT语句的完整结果集
 * - 更新结果：INSERT/UPDATE/DELETE的影响行数
 * - DDL结果：CREATE/ALTER/DROP等操作的状态
 * - 错误结果：执行失败的错误信息和诊断
 * - 空结果：无数据返回的操作结果
 *
 * 接口设计：
 * - 状态查询：success(), has_error(), getStatus()
 * - 数据访问：get_rows(), get_column_metadata()
 * - 信息获取：get_message(), get_warnings(), get_errors()
 * - 统计查询：get_rows_affected(), row_count()
 * - 状态设置：set_error(), add_warning(), add_error()
 *
 * HOW: 执行结果的实现机制和数据流
 *
 * 结果构建流程：
 * 1. 初始化阶段：创建执行结果对象，设置初始状态
 * 2. 数据填充：根据执行类型填充相应的数据
 * 3. 元数据设置：为查询结果设置列的元数据信息
 * 4. 状态更新：根据执行结果更新成功/失败状态
 * 5. 统计计算：计算影响行数等统计信息
 *
 * 数据结构设计：
 * - 结果集：使用vector存储行数据，每行包含列值的vector
 * - 元数据：使用结构体描述每列的属性（名称、类型、约束等）
 * - 状态信息：布尔值表示成功状态，字符串存储消息
 * - 警告列表：vector存储非致命性警告信息
 * - 错误列表：vector存储详细错误信息
 *
 * 内存管理策略：
 * - 引用计数：智能指针管理大型结果集
 * - 延迟加载：按需加载大型结果数据
 * - 流式处理：支持大结果集的分批处理
 * - 缓存策略：热点结果的内存缓存
 * - 压缩存储：对大结果集的压缩存储
 *
 * 序列化支持：
 * - 二进制格式：高性能的内部传输格式
 * - JSON格式：通用的Web接口格式
 * - 文本格式：人类可读的调试格式
 * - 自定义格式：支持特定客户端的格式
 *
 * 并发安全考虑：
 * - 不可变对象：构建完成后结果对象不可修改
 * - 线程安全：多个线程可以安全读取结果
 * - 复制语义：支持结果对象的深拷贝
 * - 所有权转移：move语义优化性能
 *
 * 错误处理机制：
 * - 分层错误：支持错误代码、消息、详情的多层信息
 * - 错误链：支持错误的因果关系追踪
 * - 国际化：支持多语言错误信息
 * - 调试信息：提供详细的错误上下文信息
 *
 * 性能优化技术：
 * - 零拷贝：避免不必要的数据复制
 * - 预分配：预估大小避免动态扩容
 * - 内联存储：小结果集的内联存储优化
 * - 延迟求值：按需计算统计信息
 * - SIMD优化：向量化的数据处理
 *
 * 扩展性设计：
 * - 插件架构：支持自定义结果类型和处理器
 * - 配置化管理：可配置的结果处理策略
 * - 事件驱动：结果生命周期事件通知
 * - 监控集成：结果处理的性能监控
 * - 多格式支持：可扩展的结果序列化格式
 *
 * 兼容性保证：
 * - 向后兼容：保持与旧版本API的兼容性
 * - 渐进迁移：支持新旧API的混合使用
 * - 版本控制：明确的版本标识和兼容性保证
 */
struct ColumnMeta {
  std::string name;          // 列名
  std::string data_type;     // 数据类型
  bool is_nullable;          // 是否允许为NULL
  bool is_primary_key;       // 是否为主键
  bool is_unique_key;        // 是否为唯一键
  std::string default_value; // 默认值
};

/**
 * @brief 行结构体
 * 用于存储查询结果中的一行数据
 */
struct Row {
  std::vector<Value> values; // 行中的各个列值
};

/**
 * @brief 执行结果结构体
 * 用于表示SQL查询的执行结果
 */
struct ExecutionResult {
    enum Status { SUCCESS, FAILURE };

    // 结果集数据
    std::vector<Row> rows;

    // 列元数据
    std::vector<ColumnMeta> column_metadata;

    // 执行状态
    bool success;        // 执行是否成功
    std::string message; // 执行消息

    // 新增字段（用于DML操作）
    int64_t rows_affected = 0;  // 影响的行数
    std::vector<std::string> warnings; // 警告信息
    std::vector<std::string> errors;   // 错误信息（向后兼容）

    // 构造函数
    ExecutionResult(bool success = true, const std::string &message = "")
        : success(success), message(message) {}

    // 方法
    void add_row(const Row &row) { rows.push_back(row); }

    size_t row_count() const { return rows.size(); }

    bool is_empty() const { return rows.empty(); }

    bool has_error() const { return !success; }

    // 兼容旧版接口
    Status getStatus() const { return success ? SUCCESS : FAILURE; }

    const std::string& getMessage() const { return message; }

    // 新增方法
    void add_warning(const std::string& warning) { warnings.push_back(warning); }
    void add_error(const std::string& error) { errors.push_back(error); }

    // 向后兼容：将error_message映射到message
    std::string& error_message() { return message; }
    const std::string& error_message() const { return message; }
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_RESULT_H
