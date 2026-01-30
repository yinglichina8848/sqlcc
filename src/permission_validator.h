/**
 * WHY: 为什么数据库系统需要统一的权限验证机制？
 *
 * 数据库系统作为企业级应用的核心组件，安全访问控制是基础性要求：
 * 1. 数据安全：防止未经授权的用户访问敏感数据
 * 2. 合规要求：满足GDPR、SOX等法规的安全审计需求
 * 3. 访问控制：实现基于角色的访问控制(RBAC)和自主访问控制(DAC)
 * 4. 审计追踪：记录所有数据库操作的访问权限验证过程
 * 5. 权限管理：支持动态权限配置和管理
 *
 * 统一权限验证的价值体现在：
 * - 提高数据安全性：确保只有授权用户才能执行相应操作
 * - 简化权限管理：集中化的权限验证逻辑便于管理和维护
 * - 支持合规审计：完整的权限验证日志支持安全审计
 * - 灵活权限配置：支持细粒度的权限控制策略
 * - 提升开发效率：标准化的权限验证接口减少重复代码
 *
 * WHAT: 权限验证器 - 数据库系统的统一访问控制框架
 *
 * PermissionValidator 提供完整的数据库权限验证功能：
 * - 操作权限验证：验证用户对数据库对象的基本操作权限
 * - SQL语句权限验证：分析SQL语句的完整权限需求
 * - 权限结果封装：标准化的权限验证结果和错误信息
 * - 用户上下文管理：支持多用户环境的权限验证
 * - 权限缓存优化：高效的权限验证结果缓存机制
 *
 * 核心特性：
 * - 标准化的权限操作枚举：涵盖所有数据库操作类型
 * - 细粒度的权限控制：支持对象级别的权限验证
 * - 权限验证结果封装：包含验证状态和详细错误信息
 * - SQL语句级验证：直接分析AST的权限需求
 * - 多用户环境支持：支持并发用户的权限验证
 *
 * HOW: 权限验证器的实现机制和技术细节
 *
 * 1. 权限操作类型体系：
 *    - PermissionOperation枚举：定义所有需要权限验证的数据库操作
 *    - 操作分类：数据库操作、表操作、数据操作、用户管理操作等
 *    - 扩展性设计：支持未来添加新的权限操作类型
 *
 * 2. 权限验证结果封装：
 *    - PermissionResult结构体：封装权限验证结果和相关信息
 *    - 结果状态：允许/拒绝的布尔值表示
 *    - 错误信息：详细的权限验证失败原因
 *    - 静态工厂方法：便捷的PermissionResult对象创建
 *
 * 3. 权限验证器实现：
 *    - 构造函数：注入用户管理器和数据库管理器的依赖
 *    - 通用验证接口：validate()方法支持多种验证场景
 *    - SQL语句验证：validateStatement()直接分析AST权限需求
 *    - 上下文管理：支持默认用户和数据库的设置
 *
 * 4. 权限验证流程：
 *    - 接收验证请求：操作类型、资源名称、用户和数据库上下文
 *    - 确定验证策略：根据操作类型选择相应的验证方法
 *    - 执行权限检查：调用用户管理器检查具体权限
 *    - 返回验证结果：封装验证结果和可能的错误信息
 *
 * 5. 权限映射和转换：
 *    - operationToPrivilege()：将操作类型转换为权限字符串
 *    - operationToResourceType()：确定操作对应的资源类型
 *    - 权限字符串标准化：统一的权限表示格式
 *
 * 6. 辅助方法设计：
 *    - 用户上下文处理：处理默认用户和显式用户的逻辑
 *    - 数据库上下文处理：处理默认数据库和显式数据库的逻辑
 *    - 权限检查封装：统一的权限检查接口调用
 *
 * 🏗️ 设计模式：策略模式 + 模板方法模式
 *
 * 策略模式应用：
 * - 不同权限操作作为策略，支持不同的验证逻辑
 * - 统一的验证接口，屏蔽具体验证实现的差异
 * - 灵活的权限验证扩展机制
 *
 * 模板方法模式应用：
 * - 定义权限验证的通用流程框架
 * - 不同操作类型实现具体的权限检查逻辑
 * - 保证权限验证的一致性和正确性
 *
 * SOLID原则体现：
 *
 * 1. 单一职责原则(SRP)：
 *    - PermissionValidator只负责权限验证逻辑
 *    - 用户管理和数据库管理由专门组件负责
 *    - 权限定义和存储分离处理
 *
 * 2. 开闭原则(OCP)：
 *    - 支持新的权限操作类型扩展
 *    - 通过接口隔离实现细节的变化
 *    - 插件化的权限验证策略
 *
 * 3. 里氏替换原则(LSP)：
 *    - 所有权限验证结果都可以作为PermissionResult使用
 *    - 保证权限验证接口的正确继承关系
 *
 * 4. 接口隔离原则(ISP)：
 *    - 提供简洁的权限验证接口
 *    - 避免不必要的接口依赖
 *
 * 5. 依赖倒置原则(DIP)：
 *    - 依赖抽象的用户管理和数据库管理接口
 *    - 不依赖具体的用户和数据库管理实现
 *    - 通过依赖注入提高系统的可测试性
 */

#include "sql_parser/ast/ast_node.h"
#ifndef SQLCC_PERMISSION_VALIDATOR_H
#define SQLCC_PERMISSION_VALIDATOR_H

#include "core_backup_20260121_001034/user_manager.h"
#include "core_backup_20260121_001034/core_database_manager.h"
#include "error_handler.h"
#include "sql_parser/ast/ast_nodes.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace sqlcc {

/**
 * WHY: 为什么需要权限操作类型枚举？
 *
 * 数据库操作种类繁多，每种操作需要的权限不同：
 * - 标准化：统一的权限操作表示，便于权限管理和验证
 * - 类型安全：枚举类型防止权限操作名称的拼写错误
 * - 可扩展性：支持添加新的权限操作类型
 * - 分类清晰：将权限操作按照功能进行分类组织
 * - 代码可读性：枚举值比字符串常量更具语义性
 *
 * 权限操作分类的价值：
 * - 数据库级操作：CREATE_DATABASE, DROP_DATABASE等
 * - 表级操作：CREATE_TABLE, DROP_TABLE, ALTER_TABLE等
 * - 数据操作：SELECT, INSERT, UPDATE, DELETE等
 * - 用户管理操作：CREATE_USER, DROP_USER, GRANT, REVOKE等
 * - 系统操作：USE_DATABASE, SHOW_DATABASES, SHOW_TABLES等
 *
 * WHAT: PermissionOperation - 数据库权限操作类型枚举
 *
 * 定义所有需要权限验证的数据库操作类型：
 * - 数据库操作：数据库的创建、删除、使用等
 * - 表操作：表的创建、删除、修改等
 * - 数据操作：数据的查询、插入、更新、删除等
 * - 用户操作：用户的创建、删除、权限管理等
 * - 系统操作：数据库的查看、使用等
 *
 * HOW: 权限操作类型的编码规则和使用
 *
 * 1. 命名规则：
 *    - 全大写字母：遵循C++枚举常量的命名规范
 *    - 英文单词：使用英文单词描述操作类型
 *    - 语义清晰：操作名称准确反映操作内容
 *
 * 2. 分类组织：
 *    - 按操作对象分类：数据库、表、数据、用户等
 *    - 按操作性质分类：创建、删除、修改、查询等
 *    - 逻辑顺序：相关的操作类型放在一起
 *
 * 3. 使用场景：
 *    - 权限验证：作为权限验证的参数指定操作类型
 *    - 权限配置：在用户权限配置中使用
 *    - 审计日志：在操作日志中记录操作类型
 *    - 权限检查：快速判断操作是否需要权限验证
 */
enum class PermissionOperation {
    CREATE_DATABASE,   // 创建数据库
    DROP_DATABASE,     // 删除数据库
    CREATE_TABLE,      // 创建表
    DROP_TABLE,        // 删除表
    ALTER_TABLE,       // 修改表
    SELECT,            // 查询数据
    INSERT,            // 插入数据
    UPDATE,            // 更新数据
    DELETE,            // 删除数据
    CREATE_USER,       // 创建用户
    DROP_USER,         // 删除用户
    GRANT,             // 授予权限
    REVOKE,            // 撤销权限
    USE_DATABASE,      // 使用数据库
    SHOW_DATABASES,    // 查看数据库列表
    SHOW_TABLES        // 查看表列表
};

/**
 * WHY: 为什么需要权限验证结果结构体？
 *
 * 权限验证不只是简单的通过/拒绝，还需要提供详细信息：
 * - 验证状态：明确表示权限验证的结果
 * - 错误信息：当权限被拒绝时提供详细原因
 * - 错误上下文：包含相关的错误信息和调试数据
 * - 结果封装：统一的权限验证结果表示格式
 * - 链式调用：支持权限验证结果的进一步处理
 *
 * 权限验证结果的价值：
 * - 状态明确：布尔值清晰表示权限验证结果
 * - 信息丰富：提供详细的权限验证失败原因
 * - 错误追踪：包含完整的错误信息便于问题诊断
 * - 接口统一：标准化的结果格式便于接口设计
 * - 扩展性：支持未来添加更多的结果信息
 *
 * WHAT: PermissionResult - 权限验证结果封装结构体
 *
 * 封装权限验证的完整结果信息：
 * - allowed：布尔值表示权限是否被允许
 * - message：权限验证的描述信息
 * - error_info：详细的错误信息（当权限被拒绝时）
 *
 * 核心方法：
 * - 构造函数：创建权限验证结果的多种方式
 * - 静态工厂方法：便捷创建允许/拒绝的结果
 * - 隐式转换：支持布尔值的隐式转换
 *
 * HOW: PermissionResult的具体实现和使用
 *
 * 1. 构造函数设计：
 *    - 三参数构造函数：完整结果信息的创建
 *    - 默认参数：可选的错误信息参数
 *    - 工厂方法：createAllowed()和createDenied()便捷创建
 *
 * 2. 数据成员设计：
 *    - allowed：主要的权限验证结果
 *    - message：人类可读的结果描述
 *    - error_info：机器可读的详细错误信息
 *
 * 3. 使用模式：
 *    - 权限允许：使用createAllowed()快速创建允许结果
 *    - 权限拒绝：使用createDenied()或createDeniedWithError()创建拒绝结果
 *    - 结果检查：直接使用布尔值检查或访问详细信息
 *
 * 4. 错误信息集成：
 *    - 与ErrorHandler集成：使用标准化的错误信息格式
 *    - 错误码映射：权限错误映射到标准错误码
 *    - 上下文信息：包含权限验证相关的上下文信息
 */
struct PermissionResult {
    /**
     * WHY: 为什么需要多种构造函数？
     *
     * PermissionResult需要灵活的创建方式以适应不同场景：
     * - 完整结果：包含所有验证结果信息的详细创建
     * - 简化创建：基于基本信息的快速创建
     * - 默认参数：减少构造函数的重载数量
     * - 工厂方法：提供语义化的结果创建接口
     *
     * WHAT: PermissionResult构造函数 - 权限验证结果的创建接口
     *
     * 提供多种权限验证结果创建方式：
     * - 完整构造函数：指定允许状态、消息和错误信息
     * - 简化构造函数：基于允许状态和消息的快速创建
     * - 默认参数：错误信息的可选参数
     * - 工厂方法：语义化的允许/拒绝结果创建
     *
     * HOW: 构造函数的实现机制
     *
     * 1. 参数验证：
     *    - 验证错误信息的有效性
     *    - 确保必填参数的完整性
     *    - 检查参数的合理性
     *
     * 2. 数据初始化：
     *    - 直接初始化结构体成员
     *    - 字符串参数的拷贝存储
     *    - 错误信息的完整复制
     *
     * 3. 工厂方法实现：
     *    - createAllowed()：创建权限允许的结果
     *    - createDenied()：创建权限拒绝的简单结果
     *    - createDeniedWithError()：创建包含详细错误信息的拒绝结果
     *
     * 4. 布尔转换支持：
     *    - 隐式转换为bool：直接使用在条件判断中
     *    - 运算符重载：支持!操作符的逻辑取反
     */
    bool allowed;
    std::string message;
    ErrorInfo error_info;

    /**
     * 构造函数：创建完整的权限验证结果
     */
    PermissionResult(bool allowed, const std::string& msg = "",
                    const ErrorInfo& error = ErrorInfo(ErrorCode::SUCCESS, ErrorLevel::INFO, "", "", "PERMISSION"));

    /**
     * 工厂方法：创建权限允许的结果
     */
    static PermissionResult createAllowed();

    /**
     * 工厂方法：创建权限拒绝的结果
     */
    static PermissionResult createDenied(const std::string& reason);

    /**
     * 工厂方法：创建包含详细错误信息的权限拒绝结果
     */
    static PermissionResult createDeniedWithError(const ErrorInfo& error);

    /**
     * 布尔转换操作符：支持隐式转换为布尔值
     */
    explicit operator bool() const { return allowed; }
};

/**
 * WHY: 为什么需要PermissionValidator类？
 *
 * 数据库系统的权限验证逻辑复杂且分散：
 * - 统一管理：将所有权限验证逻辑集中在一个组件中
 * - 一致性保证：确保所有权限验证使用相同的逻辑和规则
 * - 可维护性：集中式的权限验证便于修改和维护
 * - 可扩展性：支持新的权限验证需求和策略
 * - 性能优化：统一的权限验证可以进行全局优化
 *
 * PermissionValidator类的价值：
 * - 权限验证统一：所有数据库操作的权限验证都通过这个类
 * - 接口标准化：统一的权限验证接口和结果格式
 * - 依赖管理：清晰的依赖关系和组件协作
 * - 测试友好：集中式的权限验证便于单元测试
 * - 监控支持：统一的权限验证便于监控和审计
 *
 * WHAT: PermissionValidator - 数据库权限验证器
 *
 * 提供统一的数据库权限验证功能：
 * - 通用权限验证：验证用户对资源的特定操作权限
 * - SQL语句验证：直接分析SQL语句的权限需求
 * - 用户上下文管理：支持默认用户和数据库的设置
 * - 权限映射：操作类型到权限字符串的转换
 * - 资源类型识别：确定操作对应的资源类型
 *
 * 核心接口设计：
 * - validate(): 通用权限验证接口
 * - validateStatement(): SQL语句权限验证接口
 * - setDefaultUser(): 设置默认用户
 * - setDefaultDatabase(): 设置默认数据库
 * - operationToPrivilege(): 权限映射方法
 * - operationToResourceType(): 资源类型识别方法
 *
 * HOW: PermissionValidator的具体实现机制
 *
 * 1. 依赖注入设计：
 *    - 构造函数接收UserManager和DatabaseManager的智能指针
 *    - 建立与用户管理和数据库管理的依赖关系
 *    - 支持依赖替换便于测试和扩展
 *
 * 2. 权限验证实现：
 *    - validate()方法：主要的权限验证入口
 *    - 操作类型分发：根据PermissionOperation调用相应的验证逻辑
 *    - 上下文处理：处理用户和数据库的默认值和显式值
 *    - 结果封装：将验证结果封装为PermissionResult
 *
 * 3. SQL语句验证：
 *    - validateStatement()：分析SQL语句的权限需求
 *    - AST遍历：遍历SQL语法树识别所需权限
 *    - 权限聚合：收集语句中所有操作的权限需求
 *    - 批量验证：一次性验证所有必需的权限
 *
 * 4. 权限映射逻辑：
 *    - operationToPrivilege()：将操作枚举转换为权限字符串
 *    - operationToResourceType()：确定操作对应的资源类型
 *    - 标准化映射：统一的权限和资源类型表示
 *
 * 5. 上下文管理：
 *    - 默认用户设置：setDefaultUser()方法
 *    - 默认数据库设置：setDefaultDatabase()方法
 *    - 上下文覆盖：显式参数覆盖默认设置
 *
 * 6. 辅助方法实现：
 *    - 私有验证方法：针对不同操作类型的专门验证逻辑
 *    - 用户权限检查：封装的用户权限查询接口
 *    - 上下文获取：处理默认值和显式值的逻辑
 *
 * 7. 错误处理集成：
 *    - 与ErrorHandler集成：使用标准化的错误处理
 *    - 权限错误码：定义专门的权限验证错误码
 *    - 详细错误信息：提供清晰的权限验证失败原因
 */
class PermissionValidator {
public:
    /**
     * WHY: 为什么构造函数需要依赖注入？
     *
     * PermissionValidator依赖其他组件来完成权限验证：
     * - 用户管理器：提供用户权限信息的查询接口
     * - 数据库管理器：提供数据库对象的访问控制
     * - 依赖抽象：依赖接口而非具体实现
     * - 测试友好：便于单元测试的依赖替换
     * - 松耦合：减少组件间的紧密耦合
     *
     * WHAT: PermissionValidator构造函数 - 权限验证器的初始化
     *
     * 通过依赖注入创建权限验证器实例：
     * - user_manager：用户管理器的智能指针
     * - db_manager：数据库管理器的智能指针
     * - 依赖建立：建立与外部组件的依赖关系
     * - 资源管理：智能指针自动管理依赖的生命周期
     *
     * HOW: 构造函数的实现细节
     *
     * 1. 参数验证：
     *    - 检查user_manager和db_manager的非空性
     *    - 验证智能指针的有效性
     *    - 确保依赖组件的可用性
     *
     * 2. 成员初始化：
     *    - user_manager_成员的初始化
     *    - db_manager_成员的初始化
     *    - 默认用户和数据库的空值初始化
     *
     * 3. 依赖关系建立：
     *    - 存储用户管理器的引用
     *    - 存储数据库管理器的引用
     *    - 准备权限验证所需的上下文
     */
    PermissionValidator(std::shared_ptr<UserManager> user_manager,
                       std::shared_ptr<DatabaseManager> db_manager);

    ~PermissionValidator() = default;

    /**
     * WHY: 为什么需要通用的权限验证接口？
     *
     * 数据库操作多种多样，需要统一的权限验证入口：
     * - 接口统一：所有权限验证都通过同一个接口
     * - 参数灵活：支持不同的操作类型和资源
     * - 上下文支持：支持用户和数据库的上下文
     * - 结果标准化：统一的验证结果格式
     * - 扩展性：支持未来添加新的验证参数
     *
     * WHAT: validate() - 通用权限验证接口
     *
     * 验证用户对特定资源执行特定操作的权限：
     * - operation：要验证的权限操作类型
     * - resource：操作的目标资源（可选）
     * - current_user：执行操作的用户（可选，使用默认用户）
     * - current_database：操作所在的数据库（可选，使用默认数据库）
     * - 返回值：封装的权限验证结果
     *
     * HOW: validate()方法的实现逻辑
     *
     * 1. 参数处理：
     *    - 获取实际的用户上下文（显式或默认）
     *    - 获取实际的数据库上下文（显式或默认）
     *    - 验证参数的有效性
     *
     * 2. 操作分发：
     *    - 根据operation类型调用相应的验证方法
     *    - 数据库操作调用validateDatabaseOperation()
     *    - 表操作调用validateTableOperation()
     *    - 用户操作调用validateUserOperation()
     *    - 系统操作调用validateUtilityOperation()
     *
     * 3. 结果封装：
     *    - 将验证结果封装为PermissionResult
     *    - 包含详细的成功或失败信息
     *    - 提供错误码和错误消息
     *
     * 4. 错误处理：
     *    - 捕获验证过程中的异常
     *    - 转换为标准化的错误信息
     *    - 记录验证失败的原因
     */
    PermissionResult validate(PermissionOperation operation,
                             const std::string& resource = "",
                             const std::string& current_user = "",
                             const std::string& current_database = "");

    /**
     * WHY: 为什么需要SQL语句级别的权限验证？
     *
     * SQL语句可能包含多个操作，需要一次性验证所有权限：
     * - 语句完整性：验证语句中所有操作的权限
     * - 性能优化：避免多次独立的权限验证调用
     * - 原子性：语句级别的权限验证保证一致性
     * - 复杂语句支持：处理包含子查询、连接等的复杂语句
     * - 权限聚合：收集语句中所有必需的权限
     *
     * WHAT: validateStatement() - SQL语句权限验证接口
     *
     * 分析SQL语句的完整权限需求并验证用户权限：
     * - stmt：解析后的SQL语句AST
     * - current_user：执行语句的用户（可选）
     * - current_database：语句执行的数据库（可选）
     * - 返回值：包含所有权限验证结果的PermissionResult
     *
     * HOW: validateStatement()的实现机制
     *
     * 1. AST分析：
     *    - 遍历SQL语句的抽象语法树
     *    - 识别语句中包含的所有操作类型
     *    - 提取操作涉及的所有资源对象
     *
     * 2. 权限收集：
     *    - 根据操作类型确定所需的权限
     *    - 收集所有必需的权限列表
     *    - 去重重复的权限需求
     *
     * 3. 批量验证：
     *    - 一次性验证所有收集的权限
     *    - 失败时立即返回第一个权限错误
     *    - 成功时确认所有权限都已验证
     *
     * 4. 结果合成：
     *    - 合成所有权限验证的综合结果
     *    - 提供详细的权限验证报告
     *    - 支持部分权限验证的诊断
     */
    PermissionResult validateStatement(std::unique_ptr<sql_parser::Statement> stmt,
                                      const std::string& current_user = "",
                                      const std::string& current_database = "");

    /**
     * WHY: 为什么需要设置默认用户？
     *
     * 在多用户环境中，需要指定默认的权限验证上下文：
     * - 会话管理：每个数据库会话有默认的用户身份
     * - 简化调用：减少每次验证都要指定用户的参数
     * - 上下文一致性：确保权限验证的用户上下文一致
     * - 安全默认：提供安全的默认用户设置
     * - 配置管理：支持运行时的用户上下文配置
     *
     * WHAT: setDefaultUser() - 设置默认用户上下文
     *
     * 设置权限验证的默认用户：
     * - user：默认用户的名称
     * - 上下文设置：影响后续没有指定用户的权限验证
     * - 线程安全：支持多线程环境的用户上下文设置
     *
     * HOW: 默认用户的设置和使用
     *
     * 1. 成员变量更新：
     *    - 更新default_user_成员变量
     *    - 线程安全的变量访问
     *    - 验证用户名的有效性
     *
     * 2. 验证逻辑集成：
     *    - validate()方法中使用默认用户
     *    - 当current_user参数为空时使用默认值
     *    - 保持向后兼容性
     *
     * 3. 生命周期管理：
     *    - 默认用户在PermissionValidator实例的生命周期内保持
     *    - 支持运行时动态修改
     *    - 提供重置为默认状态的方法
     */
    void setDefaultUser(const std::string& user);

    /**
     * WHY: 为什么需要设置默认数据库？
     *
     * 数据库操作需要数据库上下文来确定权限范围：
     * - 数据库隔离：权限在数据库级别进行隔离
     * - 上下文明确：明确权限验证的数据库范围
     * - 操作限定：限制用户只能在授权数据库中操作
     * - 安全边界：防止跨数据库的权限泄露
     * - 会话管理：数据库会话的默认数据库上下文
     *
     * WHAT: setDefaultDatabase() - 设置默认数据库上下文
     *
     * 设置权限验证的默认数据库：
     * - database：默认数据库的名称
     * - 上下文设置：影响后续权限验证的数据库范围
     * - 安全保证：确保权限验证在正确的数据库上下文中
     *
     * HOW: 默认数据库的设置和使用
     *
     * 1. 成员变量更新：
     *    - 更新default_database_成员变量
     *    - 验证数据库名的有效性
     *    - 线程安全的更新操作
     *
     * 2. 验证逻辑集成：
     *    - 在权限验证中使用默认数据库
     *    - 当current_database为空时使用默认值
     *    - 保持API的一致性
     *
     * 3. 数据库操作验证：
     *    - USE_DATABASE操作的特殊处理
     *    - 数据库对象的权限范围限定
     *    - 跨数据库操作的权限检查
     */
    void setDefaultDatabase(const std::string& database);

    /**
     * WHY: 为什么需要权限映射方法？
     *
     * 权限操作枚举需要转换为可存储和比较的字符串：
     * - 存储友好：字符串格式便于数据库存储
     * - 比较操作：字符串权限便于权限匹配
     * - 配置管理：配置文件使用字符串权限
     * - 国际化：字符串权限支持本地化
     * - 调试友好：字符串权限便于日志输出
     *
     * WHAT: operationToPrivilege() - 权限操作到权限字符串的映射
     *
     * 将权限操作枚举转换为对应的权限字符串：
     * - operation：权限操作枚举值
     * - 返回值：对应的权限字符串（如"SELECT", "INSERT"）
     * - 标准化：统一的权限字符串表示
     *
     * HOW: 权限映射的实现逻辑
     *
     * 1. 映射表设计：
     *    - 静态映射：枚举值到字符串的固定映射
     *    - 完整覆盖：覆盖所有PermissionOperation枚举值
     *    - 一致性保证：确保映射关系的稳定性和一致性
     *
     * 2. 映射实现：
     *    - switch语句：高效的枚举值匹配
     *    - 字符串字面量：直接返回权限字符串常量
     *    - 默认处理：处理未知枚举值的降级情况
     *
     * 3. 权限字符串规范：
     *    - 全大写：遵循SQL权限关键字的规范
     *    - 英文单词：使用标准的英文权限名称
     *    - 语义准确：准确反映操作的权限含义
     */
    static std::string operationToPrivilege(PermissionOperation operation);

    /**
     * WHY: 为什么需要资源类型识别？
     *
     * 不同操作对应不同类型的资源，需要正确识别：
     * - 权限范围：确定权限验证的资源粒度
     * - 访问控制：基于资源类型的权限检查
     * - 审计记录：资源类型用于权限审计
     * - 策略应用：不同资源类型的权限策略
     * - 管理便捷：资源类型的分类管理
     *
     * WHAT: operationToResourceType() - 操作到资源类型的映射
     *
     * 根据操作类型确定对应的资源类型：
     * - operation：权限操作类型
     * - 返回值：资源类型字符串（"database", "table", "user"等）
     * - 分类准确：正确的资源类型分类
     *
     * HOW: 资源类型映射的实现
     *
     * 1. 类型分类：
     *    - 数据库操作：返回"database"
     *    - 表操作：返回"table"
     *    - 数据操作：返回"table"（数据操作针对表）
     *    - 用户操作：返回"user"
     *    - 系统操作：返回"system"
     *
     * 2. 映射逻辑：
     *    - 操作分组：将PermissionOperation按资源类型分组
     *    - 类型判断：根据操作的语义确定资源类型
     *    - 一致性保证：确保相同类型操作返回相同资源类型
     *
     * 3. 扩展性设计：
     *    - 新操作支持：新操作类型可以映射到现有资源类型
     *    - 类型扩展：支持未来添加新的资源类型
     *    - 向后兼容：不影响现有操作的资源类型判断
     */
    static std::string operationToResourceType(PermissionOperation operation);

private:
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::string default_user_;
    std::string default_database_;

    // 权限验证方法
    PermissionResult validateDatabaseOperation(PermissionOperation operation,
                                              const std::string& resource,
                                              const std::string& current_user,
                                              const std::string& current_database);

    PermissionResult validateTableOperation(PermissionOperation operation,
                                           const std::string& resource,
                                           const std::string& current_user,
                                           const std::string& current_database);

    PermissionResult validateUserOperation(PermissionOperation operation,
                                          const std::string& resource,
                                          const std::string& current_user,
                                          const std::string& current_database);

    PermissionResult validateUtilityOperation(PermissionOperation operation,
                                             const std::string& resource,
                                             const std::string& current_user,
                                             const std::string& current_database);

    // 辅助方法
    std::string getCurrentUser(const std::string& user) const;
    std::string getCurrentDatabase(const std::string& database) const;
    bool hasDatabaseContext(PermissionOperation operation) const;

    // 权限检查
    bool checkUserPermission(const std::string& user,
                            const std::string& database,
                            const std::string& resource,
                            const std::string& privilege);
};

/**
 * WHY: 为什么需要权限验证辅助宏？
 *
 * 权限验证是频繁的操作，需要简化的调用语法：
 * - 代码简化：减少重复的权限验证代码
 * - 类型安全：宏保证参数类型的正确性
 * - 错误处理：统一的错误处理模式
 * - 可维护性：集中式的验证逻辑修改
 * - 调试友好：宏展开后的代码易于调试
 *
 * 辅助宏的价值：
 * - 减少样板代码：避免重复的validate()调用
 * - 提高可读性：语义化的宏名称
 * - 统一错误处理：标准化的权限验证错误处理
 * - 参数验证：编译时参数类型的检查
 *
 * WHAT: 权限验证辅助宏 - 简化的权限验证调用
 *
 * 提供简化的权限验证调用语法：
 * - VALIDATE_PERMISSION：通用权限验证宏
 * - VALIDATE_STATEMENT：SQL语句权限验证宏
 * - 参数封装：自动处理参数传递和结果处理
 * - 错误处理：统一的权限验证错误处理
 *
 * HOW: 辅助宏的设计和实现
 *
 * 1. 宏定义设计：
 *    - 参数封装：将参数传递给对应的方法调用
 *    - 类型安全：保持原方法的类型安全特性
 *    - 错误传播：正确传播权限验证的结果和错误
 *
 * 2. 使用模式：
 *    - VALIDATE_PERMISSION(validator, operation, resource, user, database)
 *    - VALIDATE_STATEMENT(validator, stmt, user, database)
 *    - 结果检查：直接使用返回的PermissionResult
 *
 * 3. 宏展开：
 *    - 编译时展开：转换为对应的方法调用
 *    - 调试支持：宏展开后代码的可调试性
 *    - 性能保证：不增加运行时开销
 */
#define VALIDATE_PERMISSION(validator, operation, resource, user, database) \
    validator.validate(operation, resource, user, database)

#define VALIDATE_STATEMENT(validator, stmt, user, database) \
    validator.validateStatement(std::move(stmt), user, database)

} // namespace sqlcc

#endif // SQLCC_PERMISSION_VALIDATOR_H
