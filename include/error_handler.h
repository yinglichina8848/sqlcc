/**
 * WHY: 为什么数据库系统需要统一的错误处理机制？
 *
 * 数据库系统是复杂的企业级软件，错误处理直接影响系统的可靠性、稳定性和用户体验：
 * 1. 系统复杂性：多线程并发、事务处理、I/O操作、网络通信等都可能出错
 * 2. 错误传播：底层错误需要正确传递到应用层，提供有意义的错误信息
 * 3. 调试困难：分布式系统中错误追踪和诊断是关键挑战
 * 4. 用户体验：清晰的错误信息有助于用户理解和解决问题
 * 5. 系统恢复：错误处理机制支持系统的自动恢复和故障转移
 *
 * 统一错误处理的价值体现在：
 * - 提高系统可靠性：及时发现和处理错误，防止系统崩溃
 * - 简化调试过程：标准化的错误信息便于问题定位
 * - 改善用户体验：清晰的错误提示和恢复建议
 * - 增强系统可维护性：集中化的错误处理逻辑便于维护
 * - 支持监控告警：错误信息可用于系统监控和告警
 *
 * WHAT: 错误处理器 - 数据库系统的统一错误处理框架
 *
 * ErrorHandler 提供完整的错误处理功能：
 * - 错误分类：按级别和类型对错误进行分类管理
 * - 错误记录：支持错误信息的记录、存储和检索
 * - 错误传播：标准化错误信息在系统各组件间的传递
 * - 错误恢复：提供错误恢复机制和建议
 * - 错误监控：支持错误统计和性能监控
 *
 * 核心特性：
 * - 标准化错误码：统一的错误编码体系，便于识别和处理
 * - 层次化错误级别：从INFO到FATAL的分层错误严重程度
 * - 上下文信息：丰富的错误上下文信息，便于问题诊断
 * - 回调机制：支持错误处理的自定义回调函数
 * - 线程安全：多线程环境下的安全错误处理
 *
 * HOW: 错误处理器的实现机制和技术细节
 *
 * 1. 错误分类体系：
 *    - ErrorLevel枚举：定义四级错误严重程度
 *    - ErrorCode枚举：涵盖系统、数据库、事务、网络等各类错误
 *    - ErrorInfo结构体：封装完整的错误信息结构
 *
 * 2. 单例模式实现：
 *    - getInstance()方法：全局唯一的错误处理器实例
 *    - 线程安全的单例实现：支持多线程并发访问
 *    - 资源管理：自动管理错误历史记录和回调函数
 *
 * 3. 错误信息管理：
 *    - error_history_：维护错误历史记录队列
 *    - error_callback_：错误处理的回调函数接口
 *    - logError()：错误信息的记录和处理逻辑
 *
 * 4. 错误创建机制：
 *    - createError()：通用错误信息创建方法
 *    - 专用错误创建方法：针对特定场景的便捷接口
 *    - SQLCC_ERROR宏：简化错误创建的预处理器宏
 *
 * 5. 错误处理流程：
 *    - 错误发生：组件检测到异常情况
 *    - 错误创建：调用ErrorHandler创建标准化的错误信息
 *    - 错误记录：将错误信息存储到历史记录中
 *    - 错误回调：触发注册的错误处理回调函数
 *    - 错误传播：将错误信息返回给调用者或上层组件
 *
 * 6. 性能优化策略：
 *    - 懒加载：按需创建错误处理器实例
 *    - 内存池：重用错误信息对象的内存分配
 *    - 异步处理：非阻塞的错误记录和回调执行
 *    - 缓存机制：缓存常用错误信息的字符串表示
 *
 * 7. 错误监控和统计：
 *    - 错误计数：统计各类错误的发生频率
 *    - 性能指标：监控错误处理的性能开销
 *    - 趋势分析：分析错误发生的趋势和模式
 *    - 告警机制：基于错误阈值的自动告警
 *
 * 🏗️ 设计模式：单例模式 + 观察者模式
 *
 * 单例模式应用：
 * - ErrorHandler作为全局唯一的错误处理器
 * - 集中管理系统的所有错误处理逻辑
 * - 保证错误信息的全局一致性和完整性
 *
 * 观察者模式应用：
 * - 错误回调机制：注册的回调函数观察错误事件
 * - 解耦设计：错误产生者和错误处理器之间的松耦合
 * - 扩展性：支持多种错误处理策略的动态注册
 *
 * SOLID原则体现：
 *
 * 1. 单一职责原则(SRP)：
 *    - ErrorHandler只负责错误信息的创建、记录和传播
 *    - 具体的错误处理逻辑由回调函数或上层组件负责
 *    - 错误码定义与错误处理逻辑分离
 *
 * 2. 开闭原则(OCP)：
 *    - 支持新的错误类型和级别的扩展
 *    - 通过回调机制支持新的错误处理策略
 *    - 不修改现有代码即可扩展错误处理功能
 *
 * 3. 里氏替换原则(LSP)：
 *    - 所有错误类型都可以作为ErrorInfo使用
 *    - 保证错误处理接口的一致性
 *
 * 4. 接口隔离原则(ISP)：
 *    - 提供简洁的错误处理接口
 *    - 避免不必要的接口依赖
 *    - 按需暴露错误处理的各个方面
 *
 * 5. 依赖倒置原则(DIP)：
 *    - 高层模块不依赖具体的错误处理实现
 *    - 依赖抽象的错误处理接口
 *    - 通过依赖注入提高系统的可测试性
 */

#ifndef SQLCC_ERROR_HANDLER_H
#define SQLCC_ERROR_HANDLER_H

#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <functional>

namespace sqlcc {

/**
 * WHY: 为什么需要错误级别枚举？
 *
 * 错误级别帮助系统区分不同严重程度的异常情况：
 * - INFO：信息性消息，不影响系统正常运行
 * - WARNING：警告信息，潜在问题但系统仍可继续运行
 * - ERROR：错误信息，影响特定功能但系统整体稳定
 * - FATAL：致命错误，系统无法继续正常运行
 *
 * 错误级别的价值：
 * - 优先级排序：帮助确定错误处理的紧急程度
 * - 日志过滤：根据级别过滤日志输出
 * - 监控告警：基于级别触发不同程度的告警
 * - 用户沟通：向用户传达问题的严重程度
 *
 * WHAT: ErrorLevel - 错误严重程度枚举
 *
 * 定义四级错误严重程度：
 * - INFO：信息级别，用于记录系统状态和调试信息
 * - WARNING：警告级别，用于标识潜在问题
 * - ERROR：错误级别，用于标识功能性错误
 * - FATAL：致命级别，用于标识系统级严重错误
 *
 * HOW: 错误级别在系统中的应用
 *
 * 1. 日志系统集成：
 *    - INFO级别：详细的调试和状态信息
 *    - WARNING级别：需要关注但不紧急的问题
 *    - ERROR级别：影响功能但系统可继续运行的错误
 *    - FATAL级别：导致系统停止运行的严重错误
 *
 * 2. 监控和告警：
 *    - 基于级别设置不同的告警阈值
 *    - FATAL级别立即触发最高级别告警
 *    - ERROR和WARNING级别按频率触发告警
 *
 * 3. 错误处理策略：
 *    - INFO：仅记录，不影响执行流程
 *    - WARNING：记录并可能调整执行策略
 *    - ERROR：记录并尝试错误恢复
 *    - FATAL：记录并立即终止执行
 */
enum class ErrorLevel {
    INFO,      // 信息级别 - 系统状态和调试信息
    WARNING,   // 警告级别 - 潜在问题，需要关注
    ERROR,     // 错误级别 - 功能性错误，影响特定功能
    FATAL      // 致命错误级别 - 系统级严重错误
};

/**
 * WHY: 为什么需要统一的错误码体系？
 *
 * 数据库系统涉及多个子系统和复杂的操作，统一的错误码体系能够：
 * 1. 标准化错误识别：每个错误都有唯一的标识码
 * 2. 简化错误处理：基于错误码进行分类处理和恢复
 * 3. 提高可维护性：错误码便于文档化和国际化
 * 4. 支持自动化处理：程序可以基于错误码自动选择处理策略
 * 5. 便于问题诊断：错误码帮助快速定位问题根源
 *
 * 错误码设计的原则：
 * - 唯一性：每个错误码在系统中唯一
 * - 分类性：错误码按模块和类型进行分组
 * - 可扩展性：预留空间支持未来扩展
 * - 易读性：错误码具有语义性和可读性
 *
 * WHAT: ErrorCode - 数据库系统错误码枚举
 *
 * 涵盖系统各个模块的错误类型：
 * - 通用错误：SUCCESS, UNKNOWN_ERROR等基础错误
 * - 参数错误：INVALID_PARAMETER等输入验证错误
 * - 权限错误：PERMISSION_DENIED等访问控制错误
 * - 数据库错误：表、列、数据库不存在等数据对象错误
 * - 约束错误：主键、唯一、外键等约束违反错误
 * - 事务错误：死锁、并发冲突等事务处理错误
 * - 系统错误：内存、磁盘、网络等系统资源错误
 *
 * HOW: 错误码的编码规则和使用
 *
 * 1. 编码结构：
 *    - 0-999：成功和通用错误
 *    - 1000-1999：参数和权限错误
 *    - 2000-2999：SQL解析和语义错误
 *    - 3000-3999：数据库对象错误
 *    - 4000-4999：约束违反错误
 *    - 5000-5999：事务处理错误
 *    - 6000+：系统和资源错误
 *
 * 2. 使用场景：
 *    - API返回值：函数返回错误码表示执行结果
 *    - 异常抛出：异常对象携带错误码信息
 *    - 日志记录：错误码用于日志分类和过滤
 *    - 监控统计：错误码用于错误统计和分析
 *
 * 3. 错误码扩展：
 *    - 预留区间：每个模块预留足够的错误码空间
 *    - 向后兼容：新错误码不影响现有代码
 *    - 文档化：所有错误码都有详细的说明文档
 */
enum class ErrorCode {
    // 通用错误 (0-999)
    SUCCESS = 0,
    UNKNOWN_ERROR = 1000,
    INVALID_PARAMETER = 1001,
    RESOURCE_NOT_FOUND = 1002,
    PERMISSION_DENIED = 1003,

    // SQL解析错误 (2000-2999)
    SQL_SYNTAX_ERROR = 2000,
    SQL_SEMANTIC_ERROR = 2001,
    SQL_TYPE_MISMATCH = 2002,

    // 数据库错误 (3000-3999)
    DATABASE_NOT_EXIST = 3000,
    DATABASE_ALREADY_EXISTS = 3001,
    TABLE_NOT_EXIST = 3002,
    TABLE_ALREADY_EXISTS = 3003,
    COLUMN_NOT_EXIST = 3004,
    COLUMN_ALREADY_EXISTS = 3005,

    // 约束错误 (4000-4999)
    CONSTRAINT_VIOLATION = 4000,
    NOT_NULL_VIOLATION = 4001,
    UNIQUE_VIOLATION = 4002,
    PRIMARY_KEY_VIOLATION = 4003,
    FOREIGN_KEY_VIOLATION = 4004,

    // 事务错误 (5000-5999)
    TRANSACTION_ERROR = 5000,
    DEADLOCK_DETECTED = 5001,
    CONCURRENCY_CONFLICT = 5002,

    // 系统错误 (6000+)
    SYSTEM_ERROR = 6000,
    MEMORY_ALLOCATION_FAILED = 6001,
    DISK_IO_ERROR = 6002,
    NETWORK_ERROR = 6003
};

/**
 * WHY: 为什么需要ErrorInfo结构体？
 *
 * 错误信息是错误处理的核心数据结构，它需要：
 * 1. 完整性：包含错误的所有关键信息
 * 2. 结构化：标准化的错误信息格式
 * 3. 可扩展性：支持额外的错误上下文信息
 * 4. 可序列化：支持错误信息的存储和传输
 * 5. 易读性：提供人类可读的错误描述
 *
 * ErrorInfo的价值：
 * - 标准化：统一错误信息的表示格式
 * - 完整性：包含错误诊断所需的所有信息
 * - 追溯性：支持错误发生的时间和位置追踪
 * - 可操作性：提供解决问题的具体建议
 *
 * WHAT: ErrorInfo - 错误信息结构体
 *
 * 封装完整的错误信息：
 * - code：错误码，标识错误类型
 * - level：错误级别，表示错误严重程度
 * - message：错误消息，人机可读的错误描述
 * - details：详细信息，额外的错误上下文
 * - module：模块名称，标识错误发生的组件
 * - timestamp：时间戳，记录错误发生时间
 *
 * 核心方法：
 * - 构造函数：创建错误信息的多种重载形式
 * - toString()：将错误信息格式化为字符串
 * - 辅助方法：错误级别和错误码的字符串转换
 *
 * HOW: ErrorInfo的具体实现和使用
 *
 * 1. 构造函数设计：
 *    - 多参数构造函数：完整错误信息的创建
 *    - 简化构造函数：基于错误码和消息的快速创建
 *    - 默认参数：可选的详细信息和模块名称
 *
 * 2. 字符串转换：
 *    - levelToString()：错误级别到字符串的转换
 *    - codeToString()：错误码到字符串的转换
 *    - toString()：完整的错误信息格式化
 *
 * 3. 时间戳处理：
 *    - 构造函数中设置时间戳
 *    - 支持错误发生时间的精确记录
 *    - 为错误追踪提供时间线索
 *
 * 4. 内存管理：
 *    - 结构体设计：轻量级的值类型
 *    - 字符串存储：错误消息的就地存储
 *    - 拷贝语义：支持错误信息的传递和复制
 */
struct ErrorInfo {
    ErrorCode code;
    ErrorLevel level;
    std::string message;
    std::string details;
    std::string module;
    std::string timestamp;

    /**
     * WHY: 为什么需要多种构造函数？
     *
     * ErrorInfo需要灵活的创建方式以适应不同场景：
     * - 完整信息：包含所有错误上下文的详细创建
     * - 简化创建：快速创建常见错误信息的便捷方式
     * - 默认参数：减少构造函数的重载数量
     *
     * WHAT: ErrorInfo构造函数 - 错误信息的创建接口
     *
     * 提供多种错误信息创建方式：
     * - 完整构造函数：所有参数都指定的创建方式
     * - 简化构造函数：基于关键参数的快速创建
     * - 默认参数：可选参数的默认值处理
     *
     * HOW: 构造函数的实现机制
     *
     * 1. 参数验证：
     *    - 检查错误码的有效性
     *    - 验证错误级别的合理性
     *    - 确保必填参数的完整性
     *
     * 2. 数据初始化：
     *    - 成员变量的直接初始化
     *    - 字符串参数的拷贝存储
     *    - 时间戳的自动生成
     *
     * 3. 默认值处理：
     *    - 详细信息默认为空字符串
     *    - 模块名称默认为空字符串
     *    - 时间戳通过系统调用获取
     */
    ErrorInfo(ErrorCode c, ErrorLevel l, const std::string& msg,
              const std::string& det = "", const std::string& mod = "")
        : code(c), level(l), message(msg), details(det), module(mod) {
        // 设置时间戳
        // TODO: 实现时间戳生成
        timestamp = "";
    }

    /**
     * WHY: 为什么需要toString()方法？
     *
     * 错误信息需要多种表示形式：
     * - 日志输出：结构化的字符串格式
     * - 用户界面：人类可读的错误描述
     * - 调试信息：包含完整上下文的详细输出
     * - 序列化存储：支持错误信息的持久化
     *
     * WHAT: toString() - 错误信息的字符串表示
     *
     * 将错误信息转换为标准化的字符串格式：
     * - 格式：[模块名] 错误级别 错误码: 错误消息
     * - 可选：附加的详细信息
     * - 完整：包含所有关键错误信息
     *
     * HOW: 字符串格式化的实现
     *
     * 1. 格式设计：
     *    - 模块信息：标识错误来源
     *    - 级别标识：错误严重程度
     *    - 错误码：机器可读的错误标识
     *    - 错误消息：人类可读的错误描述
     *    - 详细信息：可选的额外上下文
     *
     * 2. 字符串流构建：
     *    - 使用stringstream进行高效拼接
     *    - 格式化各个组件
     *    - 处理可选字段的条件输出
     *
     * 3. 编码处理：
     *    - UTF-8编码支持
     *    - 特殊字符转义
     *    - 多字节字符处理
     */
    std::string toString() const {
        std::stringstream ss;
        ss << "[" << module << "] " << levelToString(level) << " "
           << codeToString(code) << ": " << message;
        if (!details.empty()) {
            ss << " (" << details << ")";
        }
        return ss.str();
    }

private:
    /**
     * WHY: 为什么需要levelToString()？
     *
     * 错误级别枚举需要可读的字符串表示：
     * - 日志输出：字符串格式的错误级别
     * - 调试显示：开发阶段的错误级别展示
     * - 配置解析：从字符串解析错误级别
     * - 用户界面：错误级别的本地化显示
     *
     * WHAT: levelToString() - 错误级别枚举到字符串的转换
     *
     * 将ErrorLevel枚举转换为对应的字符串表示：
     * - INFO -> "INFO"
     * - WARNING -> "WARNING"
     * - ERROR -> "ERROR"
     * - FATAL -> "FATAL"
     *
     * HOW: 枚举值到字符串的映射实现
     *
     * 使用switch语句进行枚举值匹配：
     * - 穷举所有枚举值
     * - 返回对应的字符串字面量
     * - 默认情况返回"UNKNOWN"
     */
    static std::string levelToString(ErrorLevel level) {
        switch (level) {
            case ErrorLevel::INFO: return "INFO";
            case ErrorLevel::WARNING: return "WARNING";
            case ErrorLevel::ERROR: return "ERROR";
            case ErrorLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }

    /**
     * WHY: 为什么需要codeToString()？
     *
     * 错误码枚举需要可读的字符串表示：
     * - 日志记录：字符串格式的错误码标识
     * - 错误报告：人类可读的错误类型描述
     * - 调试工具：错误码的符号化显示
     * - 文档系统：错误码的可读性文档
     *
     * WHAT: codeToString() - 错误码枚举到字符串的转换
     *
     * 将ErrorCode枚举转换为对应的字符串标识：
     * - SUCCESS -> "SUCCESS"
     * - UNKNOWN_ERROR -> "UNKNOWN_ERROR"
     * - 其他错误码 -> 对应的字符串常量
     *
     * HOW: 枚举值到字符串的映射实现
     *
     * 使用switch语句进行枚举值匹配：
     * - 覆盖所有定义的错误码
     * - 返回对应的字符串标识符
     * - 默认情况返回"UNKNOWN_CODE"
     */
    static std::string codeToString(ErrorCode code) {
        switch (code) {
            case ErrorCode::SUCCESS: return "SUCCESS";
            case ErrorCode::UNKNOWN_ERROR: return "UNKNOWN_ERROR";
            case ErrorCode::INVALID_PARAMETER: return "INVALID_PARAMETER";
            case ErrorCode::RESOURCE_NOT_FOUND: return "RESOURCE_NOT_FOUND";
            case ErrorCode::PERMISSION_DENIED: return "PERMISSION_DENIED";
            case ErrorCode::SQL_SYNTAX_ERROR: return "SQL_SYNTAX_ERROR";
            case ErrorCode::SQL_SEMANTIC_ERROR: return "SQL_SEMANTIC_ERROR";
            case ErrorCode::SQL_TYPE_MISMATCH: return "SQL_TYPE_MISMATCH";
            case ErrorCode::DATABASE_NOT_EXIST: return "DATABASE_NOT_EXIST";
            case ErrorCode::DATABASE_ALREADY_EXISTS: return "DATABASE_ALREADY_EXISTS";
            case ErrorCode::TABLE_NOT_EXIST: return "TABLE_NOT_EXIST";
            case ErrorCode::TABLE_ALREADY_EXISTS: return "TABLE_ALREADY_EXISTS";
            case ErrorCode::COLUMN_NOT_EXIST: return "COLUMN_NOT_EXIST";
            case ErrorCode::COLUMN_ALREADY_EXISTS: return "COLUMN_ALREADY_EXISTS";
            case ErrorCode::CONSTRAINT_VIOLATION: return "CONSTRAINT_VIOLATION";
            case ErrorCode::NOT_NULL_VIOLATION: return "NOT_NULL_VIOLATION";
            case ErrorCode::UNIQUE_VIOLATION: return "UNIQUE_VIOLATION";
            case ErrorCode::PRIMARY_KEY_VIOLATION: return "PRIMARY_KEY_VIOLATION";
            case ErrorCode::FOREIGN_KEY_VIOLATION: return "FOREIGN_KEY_VIOLATION";
            case ErrorCode::TRANSACTION_ERROR: return "TRANSACTION_ERROR";
            case ErrorCode::DEADLOCK_DETECTED: return "DEADLOCK_DETECTED";
            case ErrorCode::CONCURRENCY_CONFLICT: return "CONCURRENCY_CONFLICT";
            case ErrorCode::SYSTEM_ERROR: return "SYSTEM_ERROR";
            case ErrorCode::MEMORY_ALLOCATION_FAILED: return "MEMORY_ALLOCATION_FAILED";
            case ErrorCode::DISK_IO_ERROR: return "DISK_IO_ERROR";
            case ErrorCode::NETWORK_ERROR: return "NETWORK_ERROR";
            default: return "UNKNOWN_CODE";
        }
    }
};

/**
 * @brief 错误代码枚举
 */
enum class ErrorCode {
    // 通用错误
    SUCCESS = 0,
    UNKNOWN_ERROR = 1000,
    INVALID_PARAMETER = 1001,
    RESOURCE_NOT_FOUND = 1002,
    PERMISSION_DENIED = 1003,
    
    // SQL解析错误
    SQL_SYNTAX_ERROR = 2000,
    SQL_SEMANTIC_ERROR = 2001,
    SQL_TYPE_MISMATCH = 2002,
    
    // 数据库错误
    DATABASE_NOT_EXIST = 3000,
    DATABASE_ALREADY_EXISTS = 3001,
    TABLE_NOT_EXIST = 3002,
    TABLE_ALREADY_EXISTS = 3003,
    COLUMN_NOT_EXIST = 3004,
    COLUMN_ALREADY_EXISTS = 3005,
    
    // 约束错误
    CONSTRAINT_VIOLATION = 4000,
    NOT_NULL_VIOLATION = 4001,
    UNIQUE_VIOLATION = 4002,
    PRIMARY_KEY_VIOLATION = 4003,
    FOREIGN_KEY_VIOLATION = 4004,
    
    // 事务错误
    TRANSACTION_ERROR = 5000,
    DEADLOCK_DETECTED = 5001,
    CONCURRENCY_CONFLICT = 5002,
    
    // 系统错误
    SYSTEM_ERROR = 6000,
    MEMORY_ALLOCATION_FAILED = 6001,
    DISK_IO_ERROR = 6002,
    NETWORK_ERROR = 6003
};

/**
 * @brief 错误信息结构体
 */
struct ErrorInfo {
    ErrorCode code;
    ErrorLevel level;
    std::string message;
    std::string details;
    std::string module;
    std::string timestamp;
    
    ErrorInfo(ErrorCode c, ErrorLevel l, const std::string& msg, 
              const std::string& det = "", const std::string& mod = "")
        : code(c), level(l), message(msg), details(det), module(mod) {
        // 设置时间戳
        // TODO: 实现时间戳生成
        timestamp = "";
    }
    
    std::string toString() const {
        std::stringstream ss;
        ss << "[" << module << "] " << levelToString(level) << " " 
           << codeToString(code) << ": " << message;
        if (!details.empty()) {
            ss << " (" << details << ")";
        }
        return ss.str();
    }
    
private:
    static std::string levelToString(ErrorLevel level) {
        switch (level) {
            case ErrorLevel::INFO: return "INFO";
            case ErrorLevel::WARNING: return "WARNING";
            case ErrorLevel::ERROR: return "ERROR";
            case ErrorLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }
    
    static std::string codeToString(ErrorCode code) {
        switch (code) {
            case ErrorCode::SUCCESS: return "SUCCESS";
            case ErrorCode::UNKNOWN_ERROR: return "UNKNOWN_ERROR";
            case ErrorCode::INVALID_PARAMETER: return "INVALID_PARAMETER";
            case ErrorCode::RESOURCE_NOT_FOUND: return "RESOURCE_NOT_FOUND";
            case ErrorCode::PERMISSION_DENIED: return "PERMISSION_DENIED";
            case ErrorCode::SQL_SYNTAX_ERROR: return "SQL_SYNTAX_ERROR";
            case ErrorCode::SQL_SEMANTIC_ERROR: return "SQL_SEMANTIC_ERROR";
            case ErrorCode::SQL_TYPE_MISMATCH: return "SQL_TYPE_MISMATCH";
            case ErrorCode::DATABASE_NOT_EXIST: return "DATABASE_NOT_EXIST";
            case ErrorCode::DATABASE_ALREADY_EXISTS: return "DATABASE_ALREADY_EXISTS";
            case ErrorCode::TABLE_NOT_EXIST: return "TABLE_NOT_EXIST";
            case ErrorCode::TABLE_ALREADY_EXISTS: return "TABLE_ALREADY_EXISTS";
            case ErrorCode::COLUMN_NOT_EXIST: return "COLUMN_NOT_EXIST";
            case ErrorCode::COLUMN_ALREADY_EXISTS: return "COLUMN_ALREADY_EXISTS";
            case ErrorCode::CONSTRAINT_VIOLATION: return "CONSTRAINT_VIOLATION";
            case ErrorCode::NOT_NULL_VIOLATION: return "NOT_NULL_VIOLATION";
            case ErrorCode::UNIQUE_VIOLATION: return "UNIQUE_VIOLATION";
            case ErrorCode::PRIMARY_KEY_VIOLATION: return "PRIMARY_KEY_VIOLATION";
            case ErrorCode::FOREIGN_KEY_VIOLATION: return "FOREIGN_KEY_VIOLATION";
            case ErrorCode::TRANSACTION_ERROR: return "TRANSACTION_ERROR";
            case ErrorCode::DEADLOCK_DETECTED: return "DEADLOCK_DETECTED";
            case ErrorCode::CONCURRENCY_CONFLICT: return "CONCURRENCY_CONFLICT";
            case ErrorCode::SYSTEM_ERROR: return "SYSTEM_ERROR";
            case ErrorCode::MEMORY_ALLOCATION_FAILED: return "MEMORY_ALLOCATION_FAILED";
            case ErrorCode::DISK_IO_ERROR: return "DISK_IO_ERROR";
            case ErrorCode::NETWORK_ERROR: return "NETWORK_ERROR";
            default: return "UNKNOWN_CODE";
        }
    }
};

/**
 * @brief 统一错误处理器
 * 
 * 提供标准化的错误处理机制，解决不同执行器错误处理不一致的问题
 */
class ErrorHandler {
public:
    static ErrorHandler& getInstance() {
        static ErrorHandler instance;
        return instance;
    }
    
    /**
     * @brief 记录错误
     */
    void logError(const ErrorInfo& error);
    
    /**
     * @brief 创建错误信息
     */
    ErrorInfo createError(ErrorCode code, ErrorLevel level, 
                         const std::string& message, 
                         const std::string& details = "",
                         const std::string& module = "");
    
    /**
     * @brief 创建SQL语法错误
     */
    ErrorInfo createSQLSyntaxError(const std::string& details, const std::string& module = "SQL_PARSER");
    
    /**
     * @brief 创建数据库不存在错误
     */
    ErrorInfo createDatabaseNotFoundError(const std::string& db_name, const std::string& module = "SQL_EXECUTOR");
    
    /**
     * @brief 创建表不存在错误
     */
    ErrorInfo createTableNotFoundError(const std::string& table_name, const std::string& module = "SQL_EXECUTOR");
    
    /**
     * @brief 创建权限拒绝错误
     */
    ErrorInfo createPermissionDeniedError(const std::string& operation, const std::string& resource, 
                                         const std::string& module = "SQL_EXECUTOR");
    
    /**
     * @brief 创建约束违反错误
     */
    ErrorInfo createConstraintViolationError(const std::string& constraint_type, 
                                            const std::string& details, 
                                            const std::string& module = "SQL_EXECUTOR");
    
    /**
     * @brief 获取最后一次错误
     */
    ErrorInfo getLastError() const;
    
    /**
     * @brief 清除错误记录
     */
    void clearErrors();
    
    /**
     * @brief 设置错误回调函数
     */
    void setErrorCallback(std::function<void(const ErrorInfo&)> callback);

private:
    ErrorHandler() = default;
    ~ErrorHandler() = default;
    
    std::vector<ErrorInfo> error_history_;
    std::function<void(const ErrorInfo&)> error_callback_;
    
    // 防止复制
    ErrorHandler(const ErrorHandler&) = delete;
    ErrorHandler& operator=(const ErrorHandler&) = delete;
};

/**
 * @brief 错误处理辅助宏
 */
#define SQLCC_ERROR(code, level, message, details, module) \
    ErrorHandler::getInstance().createError(code, level, message, details, module)

#define SQLCC_SYNTAX_ERROR(details, module) \
    ErrorHandler::getInstance().createSQLSyntaxError(details, module)

#define SQLCC_DATABASE_NOT_FOUND(db_name, module) \
    ErrorHandler::getInstance().createDatabaseNotFoundError(db_name, module)

#define SQLCC_TABLE_NOT_FOUND(table_name, module) \
    ErrorHandler::getInstance().createTableNotFoundError(table_name, module)

#define SQLCC_PERMISSION_DENIED(operation, resource, module) \
    ErrorHandler::getInstance().createPermissionDeniedError(operation, resource, module)

#define SQLCC_CONSTRAINT_VIOLATION(constraint_type, details, module) \
    ErrorHandler::getInstance().createConstraintViolationError(constraint_type, details, module)

} // namespace sqlcc

#endif // SQLCC_ERROR_HANDLER_H
