/**
 * WHY: 为什么需要专门的连接处理器？
 *
 * 数据库服务器处理每个客户端连接时需要精细的状态管理和复杂的协议处理，传统方案存在诸多技术挑战：
 * - 消息协议解析复杂：SQL查询、响应、错误等消息的编解码和序列化处理
 * - 连接状态管理困难：认证状态、事务状态、加密状态间的复杂转换
 * - 并发处理效率低下：单个连接的处理逻辑耦合度高，难以并发优化
 * - 错误处理不统一：网络异常、协议错误、权限验证等错误处理分散
 * - 资源管理复杂：连接相关的缓冲区、会话、加密上下文等资源管理
 * - 性能监控缺失：单个连接的性能指标收集和分析手段不足
 * - 扩展性限制：新消息类型、新处理逻辑的集成困难
 *
 * 连接处理器的核心价值：
 * 1. 协议处理集中化：统一的网络消息协议解析和处理框架
 * 2. 状态机驱动处理：基于状态机的连接处理流程控制
 * 3. 异步I/O优化：高效的异步读写操作和缓冲区管理
 * 4. 安全验证集成：完整的用户认证和权限验证机制
 * 5. 加密通信支持：TLS加密和AES对称加密的完整支持
 * 6. 资源管理自动化：连接相关资源的自动分配和释放
 * 7. 性能监控和调优：连接级别的性能监控和优化手段
 *
 * 🏗️ 设计模式：状态模式(State Pattern) + 策略模式(Strategy Pattern) + 命令模式(Command Pattern)
 *
 * 连接处理器作为状态模式的应用：
 * - 状态驱动处理：连接的不同状态驱动不同的处理逻辑
 * - 状态转换明确：状态间的转换规则清晰，易于理解和维护
 * - 状态行为隔离：每个状态的处理逻辑独立封装
 * - 状态扩展灵活：新状态的添加不会影响现有状态
 * - 状态监控便利：状态变化的可观察性和可监控性
 *
 * SOLID原则体现：
 * - 单一职责：专门负责单个网络连接的处理和管理
 * - 开闭原则：新消息类型通过策略模式实现扩展
 * - 里氏替换：所有连接处理器实现都可以互相替换
 * - 接口隔离：连接处理接口精确定义处理契约
 * - 依赖倒置：依赖抽象的执行器、验证器、加密器接口
 *
 * WHAT: 连接处理器系统 - 单个网络连接的完整处理框架
 *
 * 核心功能：
 * - 网络消息处理：接收、解析和处理各种网络消息类型
 * - 协议编解码支持：消息的编码、解码和序列化处理
 * - 用户认证管理：连接级别的用户身份验证和会话管理
 * - 权限验证集成：数据库操作的权限检查和访问控制
 * - 加密通信处理：TLS和AES加密的透明处理
 * - 错误处理机制：网络和协议错误的统一处理和响应
 * - 性能监控统计：连接处理的性能指标收集和分析
 *
 * 系统组件：
 * - ConnectionHandler：核心处理器类，提供完整的连接处理功能
 * - MessageProcessor：消息处理器，负责消息的编解码处理
 * - AuthHandler：认证处理器，处理用户认证相关逻辑
 * - PermissionValidator：权限验证器，验证用户操作权限
 * - Encryptor：加密器，支持TLS和AES加密
 * - SessionManager：会话管理器，管理连接会话状态
 * - ErrorHandler：错误处理器，处理各种异常情况
 *
 * 消息处理流程：
 * - 消息接收：从网络连接接收原始字节数据
 * - 消息解码：将字节数据解码为结构化消息对象
 * - 消息验证：验证消息的完整性和合法性
 * - 消息路由：根据消息类型路由到相应的处理逻辑
 * - 业务处理：执行业务逻辑并生成响应消息
 * - 响应编码：将响应消息编码为字节数据
 * - 响应发送：通过网络连接发送响应数据
 *
 * 认证流程控制：
 * - 认证请求接收：接收客户端发送的认证请求
 * - 凭据验证处理：验证用户名和密码的正确性
 * - 会话创建关联：认证成功后创建会话并关联连接
 * - 权限初始化：初始化用户的权限和访问控制列表
 * - 认证状态维护：维护连接的认证状态和有效期
 * - 认证失败处理：认证失败时的错误响应和重试控制
 *
 * 权限验证机制：
 * - 操作类型识别：分析SQL语句的操作类型和影响范围
 * - 数据库权限检查：验证用户对数据库的访问权限
 * - 表级权限验证：验证用户对表的操作权限
 * - 字段级权限控制：验证用户对字段的访问权限
 * - 权限缓存优化：权限检查结果的缓存以提高性能
 * - 权限审计记录：权限检查操作的完整审计日志
 *
 * 加密通信处理：
 * - TLS握手管理：处理TLS连接的握手和证书验证
 * - 数据加密传输：对应用数据的加密和完整性保护
 * - 密钥交换处理：安全密钥交换和协商过程
 * - 会话密钥管理：TLS会话密钥的生成和管理
 * - 加密算法选择：根据安全策略选择合适的加密算法
 * - 加密错误处理：加密过程中的异常检测和处理
 *
 * 错误处理机制：
 * - 网络错误处理：连接断开、网络超时等网络层错误
 * - 协议错误处理：消息格式错误、版本不匹配等协议错误
 * - 认证错误处理：认证失败、权限不足等安全错误
 * - 业务错误处理：SQL执行错误、约束违反等业务错误
 * - 资源错误处理：内存不足、文件系统错误等资源错误
 * - 错误响应生成：生成合适的错误响应消息给客户端
 *
 * 资源管理策略：
 * - 缓冲区管理：读写缓冲区的动态分配和回收
 * - 连接资源管理：文件描述符、SSL上下文等资源管理
 * - 内存使用控制：连接处理过程中的内存使用限制
 * - 超时管理机制：连接空闲超时和操作超时的管理
 * - 资源清理策略：异常情况下的资源强制清理
 * - 资源使用监控：资源使用情况的实时监控
 *
 * 接口设计：
 * - 事件处理接口：处理网络事件的统一接口
 * - 消息处理接口：处理各种消息类型的接口
 * - 认证管理接口：用户认证和会话管理的接口
 * - 加密处理接口：加密通信处理的接口
 * - 监控统计接口：性能监控和统计的接口
 * - 配置管理接口：连接处理配置管理的接口
 * - 扩展插件接口：支持第三方插件扩展的接口
 *
 * HOW: 连接处理器系统的实现机制
 *
 * 状态模式实现：
 * 1. 状态接口定义：定义连接状态的通用接口和行为
 * 2. 具体状态实现：未认证、已认证、处理中、错误中等状态
 * 3. 状态转换逻辑：定义状态间的转换规则和触发条件
 * 4. 状态行为委托：不同状态下的行为委托给对应状态处理器
 * 5. 状态扩展机制：支持新连接状态的动态添加
 * 6. 状态验证逻辑：状态转换的合法性验证和错误处理
 *
 * 策略模式支撑：
 * 1. 消息处理策略：不同的消息类型使用不同的处理策略
 * 2. 认证策略选择：不同的认证方式和策略选择
 * 3. 加密策略配置：不同的加密算法和策略配置
 * 4. 权限策略定制：不同的权限验证策略和规则
 * 5. 错误处理策略：不同的错误处理和恢复策略
 * 6. 缓存策略配置：不同的缓存和优化策略配置
 *
 * 命令模式集成：
 * 1. 命令对象封装：将请求封装为命令对象
 * 2. 命令队列管理：命令的排队和顺序执行
 * 3. 命令撤销机制：支持命令的撤销和重做
 * 4. 命令历史记录：命令执行的历史记录和审计
 * 5. 异步命令执行：命令的异步执行和结果回调
 * 6. 命令优先级管理：不同优先级的命令处理
 *
 * 消息处理实现：
 * 1. 消息头解析：解析消息头信息（长度、类型、版本等）
 * 2. 消息体解码：根据消息类型解码消息体内容
 * 3. 消息验证逻辑：消息的完整性校验和安全性验证
 * 4. 消息分发机制：根据消息类型分发到对应的处理逻辑
 * 5. 响应消息构建：构建响应消息的编码和序列化
 * 6. 消息压缩支持：可选的消息压缩和解压缩处理
 *
 * 认证处理实现：
 * 1. 认证消息解析：解析客户端发送的认证消息
 * 2. 凭据验证过程：与用户管理器验证用户名密码
 * 3. 会话创建流程：认证成功后创建新的会话对象
 * 4. 权限初始化：初始化用户的权限和访问控制
 * 5. 认证令牌生成：生成认证令牌用于后续验证
 * 6. 认证日志记录：记录认证操作的审计日志
 *
 * 权限验证实现：
 * 1. SQL语句解析：解析SQL语句提取操作类型和对象
 * 2. 权限规则匹配：匹配用户权限与操作要求的规则
 * 3. 权限决策计算：计算用户是否有执行权限的决策
 * 4. 权限缓存机制：权限检查结果的缓存优化
 * 5. 权限审计记录：权限检查操作的审计日志记录
 * 6. 权限更新机制：权限变更的实时更新和同步
 *
 * 加密处理实现：
 * 1. TLS上下文管理：SSL_CTX的创建和配置管理
 * 2. 握手协议处理：完整的TLS握手协议实现
 * 3. 数据加密传输：应用数据的加密和MAC计算
 * 4. 密钥协商机制：Diffie-Hellman密钥交换实现
 * 5. 会话复用支持：TLS会话票据和复用机制
 * 6. 加密错误处理：加密过程中的异常检测和恢复
 *
 * 错误处理实现：
 * 1. 错误分类识别：对不同类型错误的分类识别
 * 2. 错误处理策略：根据错误类型选择处理策略
 * 3. 错误响应生成：生成适当的错误响应消息
 * 4. 错误日志记录：详细的错误信息记录和分析
 * 5. 错误恢复机制：可能的错误自动恢复机制
 * 6. 错误统计收集：错误发生情况的统计收集
 *
 * 资源管理实现：
 * 1. 缓冲区池管理：读写缓冲区的池化管理和复用
 * 2. 内存使用监控：连接处理过程中的内存使用监控
 * 3. 文件描述符管理：网络socket的文件描述符管理
 * 4. SSL对象生命周期：SSL连接对象的生命周期管理
 * 5. 超时管理机制：各种操作的超时检测和处理
 * 6. 资源泄漏检测：自动检测和防止资源泄漏
 *
 * 并发控制实现：
 * 1. 线程安全设计：所有公共接口都是线程安全的
 * 2. 锁粒度控制：最小化锁的持有范围和时间
 * 3. 原子操作应用：状态更新的原子操作保证
 * 4. 条件变量同步：线程间的状态变更通知
 * 5. 读写锁优化：读多写少场景的性能优化
 * 6. 死锁预防机制：避免死锁的锁顺序规范
 *
 * 性能优化实现：
 * 1. 零拷贝技术：网络数据处理的零拷贝优化
 * 2. 缓冲区复用：缓冲区的池化复用减少分配开销
 * 3. 消息预处理：消息的预解析和预处理优化
 * 4. 缓存机制应用：各种数据的缓存优化性能
 * 5. 异步处理模式：异步I/O和异步处理优化
 * 6. 性能监控统计：性能瓶颈的识别和优化
 *
 * 扩展性设计：
 * - 插件架构：支持自定义消息处理器和认证模块
 * - 多协议支持：支持不同网络协议的扩展
 * - 自定义加密：支持第三方加密算法的集成
 * - 监控扩展：支持自定义监控指标和告警规则
 * - AI增强：基于AI的异常检测和性能优化
 *
 * 调试和诊断：
 * - 消息日志追踪：详细的网络消息收发日志
 * - 状态转换调试：连接状态转换的调试信息
 * - 性能分析工具：连接处理的性能分析和诊断
 * - 错误堆栈追踪：异常情况的完整堆栈信息
 * - 流量分析工具：网络流量的捕获和分析
 * - 自动化测试：连接处理的自动化测试工具
 */

#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <string>

#include "utils/file_descriptor.h"
#include "../sql_executor.h"
#include "message_types.h"
#include "../permission_validator.h"
#include "network/encryption.h"

// Forward declarations for OpenSSL
typedef struct ssl_st SSL;

namespace sqlcc {
namespace network {
class SessionManager;
class Session;

/**
 * @brief 连接处理器类，负责单个客户端连接的处理
 *
 * ConnectionHandler类管理与单个客户端的网络连接，包括：
 * - 消息接收和处理
 * - 响应消息发送
 * - 加密通信支持
 * - 会话状态管理
 * - 连接生命周期管理
 */
class ConnectionHandler {
public:
    /**
     * @brief 构造函数
     * @param fd 文件描述符
     * @param session_manager 会话管理器
     * @param sql_executor SQL执行器
     * @param user_manager 用户管理器
     */
    ConnectionHandler(sqlcc::FileDescriptor&& fd,
                     std::shared_ptr<SessionManager> session_manager,
                     std::shared_ptr<sqlcc::SqlExecutor> sql_executor,
                     std::shared_ptr<sqlcc::UserManager> user_manager);

    /**
     * @brief 析构函数
     */
    ~ConnectionHandler();

    /**
     * @brief 设置TLS连接
     * @param ssl SSL连接对象
     * @param enabled 是否启用TLS
     */
    void SetTLS(SSL* ssl, bool enabled);

    /**
     * @brief 设置AES加密器
     * @param encryptor AES加密器
     */
    void SetAESEncryptor(std::shared_ptr<AESEncryptor> encryptor);

    /**
     * @brief 获取文件描述符
     * @return 文件描述符值
     */
    int GetFd() const;

    /**
     * @brief 检查连接是否已关闭
     * @return true表示已关闭，false表示活跃
     */
    bool IsClosed() const;

    /**
     * @brief 处理事件
     * @param events epoll事件标志
     */
    void HandleEvent(uint32_t events);

    /**
     * @brief 发送消息
     * @param message 要发送的消息
     */
    void SendMessage(const std::vector<char>& message);

    /**
     * @brief 加密消息
     * @param message 明文消息
     * @return 加密后的消息
     */
    std::vector<char> EncryptMessage(const std::vector<char>& message);

    /**
     * @brief 解密消息
     * @param message 密文消息
     * @return 解密后的消息
     */
    std::vector<char> DecryptMessage(const std::vector<char>& message);

private:
    /**
     * @brief 处理读事件
     */
    void HandleRead();

    /**
     * @brief 处理写事件
     */
    void HandleWrite();

    /**
     * @brief 处理消息
     * @param data 接收到的数据
     */
    void ProcessMessage(const std::vector<char>& data);

    /**
     * @brief 处理连接消息
     * @param data 连接消息数据
     */
    void HandleConnectMessage(const std::vector<char>& data);

    /**
     * @brief 处理认证消息
     * @param data 认证消息数据
     */
    void HandleAuthMessage(const std::vector<char>& data);

    /**
     * @brief 处理查询消息
     * @param data 查询消息数据
     */
    void HandleQueryMessage(const std::vector<char>& data);

    /**
     * @brief 处理密钥交换消息
     * @param data 密钥交换消息数据
     */
    void HandleKeyExchangeMessage(const std::vector<char>& data);

    /**
     * @brief 发送错误消息
     * @param error 错误信息
     */
    void SendErrorMessage(const std::string& error);

    /**
     * @brief 尝试立即发送数据
     * @param data 要发送的数据
     * @return true表示发送成功，false表示失败
     */
    bool TrySendImmediately(const std::vector<char>& data);

    /**
     * @brief 关闭连接
     */
    void Close();

    /**
     * @brief 分析查询操作类型
     * @param query SQL查询语句
     * @return 权限操作类型
     */
    sqlcc::PermissionOperation AnalyzeQueryOperation(const std::string& query);

    /**
     * @brief 从查询中提取数据库名
     * @param query SQL查询语句
     * @return 数据库名，空字符串表示默认数据库
     */
    std::string ExtractDatabaseFromQuery(const std::string& query);

    /**
     * @brief 从查询中提取表名
     * @param query SQL查询语句
     * @return 表名
     */
    std::string ExtractTableFromQuery(const std::string& query);

    sqlcc::FileDescriptor fd_;                              ///< 文件描述符
    std::shared_ptr<SessionManager> session_manager_;       ///< 会话管理器
    std::shared_ptr<sqlcc::SqlExecutor> sql_executor_;      ///< SQL执行器
    std::shared_ptr<sqlcc::UserManager> user_manager_;      ///< 用户管理器
    std::shared_ptr<sqlcc::PermissionValidator> permission_validator_; ///< 权限验证器
    std::shared_ptr<Session> session_;                      ///< 当前会话
    bool closed_;                                           ///< 连接关闭标志
    std::queue<std::vector<char>> write_queue_;            ///< 写队列
    std::mutex write_mutex_;                                ///< 写队列互斥锁

    // TLS相关成员
#ifdef __linux__
    SSL* ssl_;                                              ///< SSL连接对象
    bool tls_enabled_;                                      ///< TLS启用标志
#endif

    // AES加密相关成员
    std::shared_ptr<AESEncryptor> aes_encryptor_;           ///< AES加密器
};

} // namespace network
} // namespace sqlcc
