/**
 * WHY: 为什么需要专门的函数AST处理系统？
 *
 * 数据库系统需要支持丰富的函数调用和自定义函数定义，传统方案存在诸多问题：
 * - 函数语法复杂：函数调用、参数传递、返回值处理语法多样
 * - 类型检查困难：函数参数类型匹配和隐式转换难以验证
 * - 性能优化缺失：函数调用优化和内联展开机制不完善
 * - 扩展性差：难以添加新的内置函数和自定义函数类型
 * - 语义完整性：函数重载、多态等高级特性支持不足
 *
 * 函数AST处理系统的核心价值：
 * 1. 语法完整性：支持完整的SQL函数调用和定义语法
 * 2. 类型安全性：严格的函数参数类型检查和转换
 * 3. 性能优化：函数调用优化、内联展开和缓存机制
 * 4. 扩展灵活：支持内置函数和用户自定义函数
 * 5. 语义丰富：支持函数重载、多态和高级函数特性
 *
 * 🏗️ 设计模式：组合模式(Composite Pattern) + 访问者模式(Visitor Pattern)
 *
 * 函数AST作为组合模式的经典应用：
 * - 递归结构：函数调用可以嵌套，参数可以是其他函数调用
 * - 统一接口：所有函数节点实现统一的接口和行为
 * - 树形结构：函数调用形成树形结构，支持复杂表达式
 * - 动态构建：运行时动态构建函数调用和参数传递
 * - 类型推导：自动推导函数返回值类型和参数转换
 *
 * SOLID原则体现：
 * - 单一职责：函数AST类负责函数调用和定义的语法结构
 * - 开闭原则：新函数类型通过扩展现有类实现
 * - 里氏替换：函数AST子类可以替换基类使用
 * - 接口隔离：函数AST接口精确定义所需方法
 * - 依赖倒置：高层模块依赖函数AST接口而非实现
 *
 * WHAT: 函数AST处理系统 - 完整的函数调用和定义框架
 *
 * 核心功能：
 * - 函数调用解析：支持标准SQL函数调用语法和扩展语法
 * - 参数处理：完整的函数参数传递、类型转换和验证
 * - 返回值处理：函数返回值类型推导和处理机制
 * - 函数定义：支持用户自定义函数的定义和存储
 * - 函数重载：支持同名函数的多参数版本重载
 * - 内联优化：函数调用内联展开和性能优化
 *
 * 系统组件：
 * - FunctionCall：函数调用AST节点，表示函数调用表达式
 * - FunctionDefinition：函数定义AST节点，表示函数定义语句
 * - FunctionParameter：函数参数AST节点，表示参数定义和传递
 * - ReturnStatement：返回语句AST节点，表示函数返回值
 * - 函数目录：函数注册和查找的目录服务
 * - 类型系统：函数参数和返回值的类型检查系统
 *
 * 函数调用类型：
 * - 标量函数：返回单个值的函数，如ABS、UPPER等
 * - 聚合函数：对数据集进行聚合计算的函数，如SUM、COUNT等
 * - 窗口函数：在窗口范围内计算的函数，如ROW_NUMBER、RANK等
 * - 表值函数：返回表的函数，如TABLE函数和自定义表函数
 * - 系统函数：数据库系统的内置函数，如CURRENT_DATE等
 * - 用户函数：用户自定义的存储过程和函数
 *
 * 参数传递机制：
 * - 位置参数：按位置顺序传递的参数
 * - 命名参数：通过参数名指定的参数
 * - 默认参数：具有默认值的可选参数
 * - 可变参数：接受可变数量参数的函数
 * - 输出参数：用于返回值和状态传递的参数
 * - 引用传递：通过引用传递大数据对象
 *
 * 函数定义特性：
 * - 参数定义：函数参数的名称、类型和默认值
 * - 返回类型：函数返回值的类型定义
 * - 函数体：函数实现的SQL语句块
 * - 变量作用域：函数内部变量的作用域管理
 * - 异常处理：函数执行过程中的错误处理
 * - 权限控制：函数执行的权限验证机制
 *
 * 接口设计：
 * - 函数注册：将自定义函数注册到系统目录
 * - 函数查找：根据名称和参数类型查找匹配函数
 * - 调用构建：构建函数调用AST节点的工厂方法
 * - 类型验证：验证函数调用参数类型的检查接口
 * - 优化接口：函数调用优化的扩展接口
 *
 * HOW: 函数AST处理系统的实现机制
 *
 * 函数解析流程：
 * 1. 标识符识别：识别函数名称标识符
 * 2. 括号验证：验证函数调用的括号语法
 * 3. 参数解析：解析函数参数列表和类型
 * 4. 函数查找：根据名称和参数查找匹配函数定义
 * 5. 类型检查：验证参数类型和返回值类型
 * 6. AST构建：创建函数调用AST节点对象
 *
 * 参数处理实现：
 * 1. 参数解析：解析位置参数和命名参数
 * 2. 类型转换：自动进行参数类型的隐式转换
 * 3. 默认值处理：为缺少的参数应用默认值
 * 4. 验证检查：验证参数范围和约束条件
 * 5. 引用传递：处理引用传递的参数对象
 * 6. 内存管理：管理参数对象的生命周期
 *
 * 函数定义实现：
 * 1. 语法解析：解析CREATE FUNCTION语句语法
 * 2. 参数定义：解析函数参数的名称和类型
 * 3. 返回定义：解析函数返回值类型定义
 * 4. 主体解析：解析函数实现的SQL语句块
 * 5. 元数据存储：存储函数的元数据信息
 * 6. 权限设置：设置函数的执行权限
 *
 * 类型检查实现：
 * 1. 参数匹配：检查调用参数与函数参数的匹配
 * 2. 类型兼容：验证参数类型的兼容性和转换
 * 3. 重载解析：选择最佳匹配的重载函数版本
 * 4. 隐式转换：自动进行安全的类型转换
 * 5. 错误报告：详细报告类型检查错误信息
 * 6. 类型推导：推导复杂表达式的结果类型
 *
 * 性能优化策略：
 * - 函数内联：简单函数的调用内联展开
 * - 常量折叠：编译时计算常量函数结果
 * - 缓存结果：缓存确定性函数的计算结果
 * - 并行执行：并行执行独立的函数调用
 * - 向量化：对数组函数使用SIMD优化
 *
 * 内存管理策略：
 * - 参数池：复用函数参数对象的内存分配
 * - 返回值缓存：缓存函数返回值避免重复计算
 * - 作用域管理：管理函数调用栈的作用域
 - 垃圾回收：及时清理函数调用的临时对象
 * - 内存监控：监控函数调用过程中的内存使用
 *
 * 错误处理机制：
 * - 语法错误：函数调用语法的解析错误
 * - 类型错误：函数参数类型不匹配的错误
 * - 未定义错误：调用未定义函数的错误
 * - 权限错误：无权限调用函数的错误
 * - 执行错误：函数执行过程中的运行时错误
 * - 递归错误：函数递归调用过深的错误
 *
 * 扩展性设计：
 * - 插件架构：支持动态加载新的函数类型
 * - 外部函数：支持调用外部系统和语言的函数
 * - 分布式函数：支持分布式环境下的函数调用
 * - 流式函数：支持流数据处理的函数操作
 * - 机器学习函数：支持ML模型的函数接口
 *
 * 调试和诊断：
 * - 调用跟踪：详细记录函数调用栈和参数
 * - 性能分析：分析函数调用性能瓶颈
 * - 类型调试：调试复杂的类型推导过程
 * - 错误诊断：详细的函数错误诊断信息
 * - 可视化工具：函数调用图的可视化展示
 */

#ifndef SQLCC_FUNCTION_AST_H_H
#define SQLCC_FUNCTION_AST_H_H

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * @brief 函数AST节点
 *
 * 表示SQL中的函数调用和定义
 */
class FunctionAst {
public:
    // 函数类型枚举
    enum FunctionType {
        SCALAR_FUNCTION,      // 标量函数
        AGGREGATE_FUNCTION,   // 聚合函数
        WINDOW_FUNCTION,      // 窗口函数
        TABLE_FUNCTION,       // 表值函数
        SYSTEM_FUNCTION,      // 系统函数
        USER_DEFINED_FUNCTION // 用户自定义函数
    };

    // 参数传递模式
    enum ParameterMode {
        IN_PARAMETER,         // 输入参数
        OUT_PARAMETER,        // 输出参数
        INOUT_PARAMETER      // 输入输出参数
    };

    /**
     * @brief 函数参数定义
     */
    struct FunctionParameter {
        std::string name;           ///< 参数名称
        std::string type;           ///< 参数类型
        ParameterMode mode;         ///< 参数传递模式
        bool has_default;           ///< 是否有默认值
        std::string default_value;  ///< 默认值

        FunctionParameter(const std::string& n, const std::string& t,
                         ParameterMode m = IN_PARAMETER,
                         bool has_def = false, const std::string& def = "")
            : name(n), type(t), mode(m), has_default(has_def), default_value(def) {}
    };

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

    // 函数基本信息
    const std::string& get_name() const;
    void set_name(const std::string& name);

    FunctionType get_type() const;
    void set_type(FunctionType type);

    // 参数管理
    void add_parameter(const FunctionParameter& param);
    const std::vector<FunctionParameter>& get_parameters() const;
    size_t get_parameter_count() const;

    // 返回类型
    const std::string& get_return_type() const;
    void set_return_type(const std::string& return_type);

    // 函数体（对于用户定义函数）
    const std::string& get_function_body() const;
    void set_function_body(const std::string& body);

    // 函数调用构建
    static std::unique_ptr<FunctionAst> create_function_call(
        const std::string& name,
        const std::vector<std::string>& arguments);

    // 类型检查
    bool validate_parameters(const std::vector<std::string>& call_args) const;
    bool is_compatible_return_type(const std::string& expected_type) const;

    // 函数注册和查找
    static bool register_function(std::unique_ptr<FunctionAst> function);
    static const FunctionAst* find_function(const std::string& name);
    static std::vector<const FunctionAst*> find_functions_by_type(FunctionType type);

private:
    std::string name_;                          ///< 函数名称
    FunctionType type_;                         ///< 函数类型
    std::vector<FunctionParameter> parameters_; ///< 参数列表
    std::string return_type_;                   ///< 返回类型
    std::string function_body_;                 ///< 函数体（用户定义函数）
    bool initialized_;                          ///< 初始化状态

    // 静态函数注册表
    static std::unordered_map<std::string, std::unique_ptr<FunctionAst>> function_registry_;
};

} // namespace sqlcc

#endif // SQLCC_FUNCTION_AST_H_H
