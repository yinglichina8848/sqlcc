/**
 * @file network.h
 * @brief SQLCC网络通信系统 - 客户端-服务器安全通信架构
 *
 * 网络通信系统是数据库系统的"门户"，负责客户端与服务器之间的安全数据传输。
 * 本文件实现了完整的网络通信协议栈，包括SSL/TLS加密、状态机管理和消息处理流水线。
 *
 * 📚 配套教材参考：
 * - [第10章：网络通信与分布式系统](../../textbook/《数据库系统原理与开发实践》.md#第十章网络通信与分布式系统)
 * - [10.1 客户端-服务器通信协议](../../textbook/《数据库系统原理与开发实践》.md#101-客户端-服务器通信协议)
 * - [10.2 SSL/TLS安全通信实现](../../textbook/《数据库系统原理与开发实践》.md#102-ssltls安全通信实现)
 * - [10.3 连接状态机与消息处理](../../textbook/《数据库系统原理与开发实践》.md#103-连接状态机与消息处理)
 *
 * WHY层 - 设计意图：
 *   网络通信是数据库系统的安全边界，任何数据传输都必须经过严格的安全验证和完整性检查。
 *   通过精心设计的通信协议和加密机制，确保客户端与服务器之间的数据传输安全可靠，
 *   为企业级数据库应用提供坚实的安全通信基础。
 *
 * WHAT层 - 功能说明：
 *   - SSL/TLS加密：基于OpenSSL的完整加密通信支持
 *   - 连接状态机：严格的状态转换控制和安全验证
 *   - 消息处理流水线：高效的消息编解码和处理机制
 *   - 会话管理：连接生命周期和认证状态维护
 *   - 异常处理：网络故障的优雅降级和恢复机制
 *
 * HOW层 - 实现机制：
 *   - 异步I/O：基于epoll的事件驱动架构
 *   - 状态机驱动：严格的连接状态转换控制
 *   - 流水线处理：消息的阶段性处理和转换
 *   - 内存安全：RAII模式的文件描述符管理
 *   - 错误恢复：断路器模式和自动重连机制
 *
 * 安全通信策略：
 *   - **传输层安全**：SSL/TLS 1.3协议支持
 *   - **身份认证**：双向证书认证和用户名密码验证
 *   - **数据完整性**：HMAC消息认证码保证数据不被篡改
 *   - **密钥轮换**：定期更新加密密钥防止泄露
 *   - **流量控制**：防止DoS攻击和资源耗尽
 *
 * 性能优化考虑：
 *   - **零拷贝传输**：减少数据拷贝开销
 *   - **连接复用**：长连接和连接池管理
 *   - **批量处理**：消息的批量发送和接收
 *   - **自适应压缩**：基于负载的动态压缩策略
 *   - **资源限制**：防止恶意客户端资源耗尽
 *
 * 故障处理机制：
 *   - **连接恢复**：网络故障后的自动重连
 *   - **消息重传**：超时和错误消息的自动重发
 *   - **优雅降级**：部分功能故障时的降级处理
 *   - **监控告警**：实时监控和异常告警
 *   - **审计日志**：完整的安全审计和访问日志
 *
 * 扩展性设计：
 *   - **协议扩展**：支持自定义消息类型和协议
 *   - **多路复用**：单个连接支持多个逻辑会话
 *   - **负载均衡**：客户端的智能负载分发
 *   - **集群支持**：分布式数据库集群通信
 *   - **插件架构**：可插拔的加密和压缩模块
 *
 * 消息处理流水线详解：
 *   1. **接收阶段**：网络数据接收和初步验证
 *   2. **解码阶段**：消息头的解析和数据解密
 *   3. **验证阶段**：消息完整性和权限检查
 *   4. **处理阶段**：业务逻辑执行和结果生成
 *   5. **编码阶段**：响应消息的编码和加密
 *   6. **发送阶段**：网络数据的发送和确认
 *
 * 连接状态机详解：
 *   严格的状态转换图确保通信的安全性和正确性：
 *   DISCONNECTED → CONNECTING → CONNECTED → AUTHENTICATING →
 *   AUTHENTICATED → KEY_EXCHANGING → ENCRYPTED → (正常通信)
 *   任何阶段都可能因错误进入CONNECTION_ERROR或CLOSING状态
 *
 * SSL/TLS加密实现：
 *   - **证书管理**：X.509证书链的验证和存储
 *   - **密钥协商**：ECDHE密钥交换协议
 *   - **加密算法**：AES-256-GCM对称加密
 *   - **完整性保护**：SHA-256消息摘要
 *   - **会话复用**：TLS会话票据的缓存和重用
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2025-12-24
 */

#ifndef SQLCC_NETWORK_H
#define SQLCC_NETWORK_H

#include <string>
#include <memory>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>

#include "sql_executor.h"
#include "network/encryption.h"
#include "utils/file_descriptor.h"
#include "utils/ssl_wrapper.h"
#ifdef __linux__
#include <openssl/ssl.h>
#endif

namespace sqlcc {
namespace network {

// 消息类型枚举
enum MessageType {
    CONNECT = 0,        // 连接请求
    CONN_ACK = 1,       // 连接确认
    AUTH = 2,           // 认证请求
    AUTH_ACK = 3,       // 认证确认
    QUERY = 4,          // 查询请求
    QUERY_RESULT = 5,   // 查询结果
    ERROR = 6,          // 错误消息
    CLOSE = 7,          // 关闭连接
    KEY_EXCHANGE = 8,   // 密钥交换
    KEY_EXCHANGE_ACK = 9 // 密钥交换确认
};

// 连接状态枚举 - 严格状态机控制
enum ConnectionState {
    DISCONNECTED = 0,       // 未连接
    CONNECTING = 1,         // 正在连接
    CONNECTED = 2,          // 已连接但未认证
    AUTHENTICATING = 3,     // 正在认证
    AUTHENTICATED = 4,      // 已认证可以正常通信
    KEY_EXCHANGING = 5,     // 正在进行密钥交换
    ENCRYPTED = 6,          // 已加密可以安全通信
    CLOSING = 7,           // 正在关闭
    CLOSED = 8,            // 已关闭
    CONNECTION_ERROR = 9   // 连接错误状态
};

// 连接状态机类 - 严格验证连接状态转换安全性
class ConnectionStateMachine {
public:
    ConnectionStateMachine();
    ~ConnectionStateMachine() = default;

    // 状态查询
    ConnectionState GetCurrentState() const;
    bool IsInState(ConnectionState state) const;
    std::string GetStateName(ConnectionState state) const;

    // 状态转换 - 带验证的安全转换
    bool TransitionTo(ConnectionState new_state);
    bool CanTransitionTo(ConnectionState new_state) const;

    // 便捷状态检查方法
    bool IsDisconnected() const { return state_ == DISCONNECTED; }
    bool IsConnecting() const { return state_ == CONNECTING; }
    bool IsConnected() const { return state_ == CONNECTED || state_ == AUTHENTICATED || state_ == ENCRYPTED; }
    bool IsAuthenticated() const { return state_ == AUTHENTICATED || state_ == ENCRYPTED; }
    bool IsEncrypted() const { return state_ == ENCRYPTED; }
    bool IsClosed() const { return state_ == CLOSED || state_ == CLOSING; }
    bool IsError() const { return state_ == CONNECTION_ERROR; }

    // 允许的操作检查
    bool CanConnect() const;
    bool CanAuthenticate() const;
    bool CanSendQuery() const;
    bool CanEncrypt() const;
    bool CanClose() const;

    // 重置状态机
    void Reset();
    void ForceError();

    // 状态变更回调（可选）
    using StateChangeCallback = std::function<void(ConnectionState old_state, ConnectionState new_state)>;
    void SetStateChangeCallback(StateChangeCallback callback);

    // 超时和重连机制
    void SetTimeouts(std::chrono::milliseconds connect_timeout = std::chrono::seconds(30),
                    std::chrono::milliseconds reconnect_delay = std::chrono::seconds(5),
                    std::chrono::milliseconds keep_alive_interval = std::chrono::seconds(60));

    void StartConnectionTimer();
    void StopConnectionTimer();
    bool IsConnectionTimedOut() const;
    void ResetConnectionTimer();

    // 重连机制
    void EnableAutoReconnect(bool enabled, int max_retry_attempts = 3);
    bool IsAutoReconnectEnabled() const;
    int GetMaxRetryAttempts() const;
    int GetCurrentRetryCount() const;
    void IncrementRetryCount();
    void ResetRetryCount();
    bool ShouldAttemptReconnect() const;
    std::chrono::milliseconds GetReconnectDelay() const;

    // 保活机制
    void EnableKeepAlive(bool enabled);
    bool IsKeepAliveEnabled() const;
    void UpdateLastActivity();
    bool IsKeepAliveTimeout() const;

private:
    ConnectionState state_;
    mutable std::mutex mutex_;  // 线程安全
    StateChangeCallback state_change_callback_;

    // 超时和重连相关成员变量
    std::chrono::milliseconds connect_timeout_;
    std::chrono::milliseconds reconnect_delay_;
    std::chrono::milliseconds keep_alive_interval_;

    std::chrono::steady_clock::time_point connection_start_time_;
    std::chrono::steady_clock::time_point last_activity_time_;
    bool connection_timer_active_;
    bool auto_reconnect_enabled_;
    bool keep_alive_enabled_;
    int max_retry_attempts_;
    int current_retry_count_;

    // 验证状态转换的私有方法
    bool IsValidTransition(ConnectionState from, ConnectionState to) const;
};

// 消息头结构
struct MessageHeader {
    uint32_t magic;        // 魔数 'SQLC'
    uint32_t length;       // 消息体长度
    uint16_t type;         // 消息类型
    uint16_t flags;        // 标志位
    uint32_t sequence_id;  // 序列号
};

// 会话类
class Session {
public:
    Session(int session_id);
    
    int GetSessionId() const { return session_id_; }
    bool IsAuthenticated() const { return authenticated_; }
    const std::string& GetUser() const { return user_; }
    void SetAuthenticated(const std::string& user) {
        authenticated_ = true;
        user_ = user;
    }
    
    // 加密和认证控制方法
    void SetEncryptionDisabled(bool disabled);
    bool IsEncryptionDisabled() const;
    void SetAuthenticationDisabled(bool disabled);
    bool IsAuthenticationDisabled() const;
    
    // AES加密支持
    void SetAESEncryptor(std::shared_ptr<AESEncryptor> encryptor);
    std::shared_ptr<AESEncryptor> GetAESEncryptor() const;
    bool IsAESEncryptionEnabled() const;

private:
    int session_id_;
    bool authenticated_;
    std::string user_;
    bool encryption_disabled_;     // 是否禁用加密
    bool authentication_disabled_; // 是否禁用认证
    std::shared_ptr<class AESEncryptor> aes_encryptor_;  // AES加密器
};

// 会话管理器
class SessionManager {
public:
    SessionManager();
    
    std::shared_ptr<Session> CreateSession();
    std::shared_ptr<Session> GetSession(int session_id);
    void DestroySession(int session_id);
    bool Authenticate(int session_id, const std::string& username, 
                     const std::string& password);
    bool CheckPermission(int session_id, const std::string& database,
                        const std::string& operation);

private:
    std::unordered_map<int, std::weak_ptr<Session>> sessions_;
    std::mutex sessions_mutex_;
    int next_session_id_;
};

// Forward declaration for ClientConnection
class ClientConnection;

// 客户端网络管理器
class ClientNetworkManager {
public:
    ClientNetworkManager(const std::string& host, int port);
    ~ClientNetworkManager();
    
    bool Connect();
    void Disconnect();
    bool IsConnected() const;
    bool SendRequest(const std::vector<char>& request);
    std::vector<char> ReceiveResponse();
    bool ConnectAndAuthenticate(const std::string& username,
                               const std::string& password);
    bool SendAuthMessage(const std::string& username, const std::string& password);
    
    // AES加密支持
    bool InitiateKeyExchange();  // 起动密钥交换
    void SetAESEncryptor(std::shared_ptr<AESEncryptor> encryptor);
    std::shared_ptr<AESEncryptor> GetAESEncryptor() const;
    bool IsAESEncryptionEnabled() const;

    // AES 加密/解密（对测试公开）
    std::vector<char> EncryptMessage(const std::vector<char>& message);
    std::vector<char> DecryptMessage(const std::vector<char>& message);

    // TLS 客户端支持
    void EnableTLS(bool enabled);
#ifdef __linux__
    bool ConfigureTLSClient(const std::string& ca_cert_path);
#endif

private:
    // AES加密/解密实现（在实现文件中定义）
    
    std::unique_ptr<ClientConnection> connection_;
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<AESEncryptor> aes_encryptor_;  // AES加密器
};

// 连接处理器
class ConnectionHandler {
public:
    ConnectionHandler(sqlcc::FileDescriptor&& fd, std::shared_ptr<SessionManager> session_manager, std::shared_ptr<sqlcc::SqlExecutor> sql_executor);
    ~ConnectionHandler();
    
    int GetFd() const;
    bool IsClosed() const;
    void HandleEvent(uint32_t events);
    void HandleRead();  // 设为public以便NetworkServer调用
    void ProcessMessage(const std::vector<char>& data);

#ifdef __linux__
    void SetTLS(struct ssl_st* ssl, bool enabled);
#endif

private:
    void HandleWrite();
    void SendMessage(const std::vector<char>& message);
    bool TrySendImmediately(const std::vector<char>& data);
    void Close();
    
    void HandleConnectMessage(const std::vector<char>& data);
    void HandleAuthMessage(const std::vector<char>& data);
    void HandleQueryMessage(const std::vector<char>& data);
    void HandleKeyExchangeMessage(const std::vector<char>& data);
    void SendErrorMessage(const std::string& error);
    
    // AES加密半加密/解密方法
    std::vector<char> EncryptMessage(const std::vector<char>& message);
    std::vector<char> DecryptMessage(const std::vector<char>& message);
    
    sqlcc::FileDescriptor fd_;  // RAII文件描述符管理
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<sqlcc::SqlExecutor> sql_executor_;
    std::shared_ptr<Session> session_;
    bool closed_;
    std::queue<std::vector<char>> write_queue_;
    std::mutex write_mutex_;
#ifdef __linux__
    sqlcc::utils::SSLSocket ssl_;      // SSL RAII包装器
    bool tls_enabled_ = false;
#endif
};

// 消息处理器
class MessageProcessor {
public:
    MessageProcessor(std::shared_ptr<SessionManager> session_manager);
    
private:
    std::shared_ptr<SessionManager> session_manager_;
};

// 密钥轮换策略
class KeyRotationPolicy {
public:
    explicit KeyRotationPolicy(size_t interval_messages = 1000)
        : interval_(interval_messages) {}
    bool ShouldRotate(size_t messages_sent) const { return interval_ > 0 && (messages_sent % interval_) == 0; }
private:
    size_t interval_;
};

// 数据传输边界检查器 - 防止缓冲区溢出和数据包完整性问题
class DataTransmissionValidator {
public:
    DataTransmissionValidator();
    ~DataTransmissionValidator() = default;

    // 数据包完整性验证
    bool ValidateMessageHeader(const MessageHeader& header) const;
    bool ValidateMessageLength(size_t declared_length, size_t actual_length) const;
    bool ValidateMessageMagic(uint32_t magic) const;
    bool ValidateMessageType(uint16_t type) const;

    // 缓冲区边界检查
    bool IsBufferSizeValid(size_t buffer_size) const;
    bool IsMessageSizeWithinLimits(size_t message_size) const;
    size_t GetMaxMessageSize() const;
    size_t GetMaxBufferSize() const;

    // 大数据包分片处理
    bool ShouldFragmentMessage(size_t message_size) const;
    std::vector<std::vector<char>> FragmentMessage(const std::vector<char>& message);
    bool ValidateFragment(const std::vector<char>& fragment) const;
    std::vector<char> ReassembleFragments(const std::vector<std::vector<char>>& fragments);

    // 流量控制
    bool CanAcceptMessage(size_t message_size, std::chrono::milliseconds time_window);
    void RecordMessageSent(size_t message_size);
    void RecordMessageReceived(size_t message_size);
    double GetCurrentThroughput() const; // 字节/秒
    bool IsRateLimited() const;

    // 配置参数
    void SetMaxMessageSize(size_t max_size);
    void SetMaxBufferSize(size_t max_size);
    void SetFragmentSize(size_t fragment_size);
    void SetRateLimit(size_t bytes_per_second);

private:
    // 配置参数
    size_t max_message_size_;      // 最大消息大小 (默认64MB)
    size_t max_buffer_size_;       // 最大缓冲区大小 (默认128MB)
    size_t fragment_size_;         // 分片大小 (默认1MB)
    size_t rate_limit_bytes_per_sec_; // 速率限制 (默认100MB/s)

    // 流量控制状态
    mutable std::mutex traffic_mutex_;
    std::deque<std::pair<std::chrono::steady_clock::time_point, size_t>> sent_messages_;
    std::deque<std::pair<std::chrono::steady_clock::time_point, size_t>> received_messages_;
    size_t total_bytes_sent_;
    size_t total_bytes_received_;

    // 魔数验证
    static constexpr uint32_t EXPECTED_MAGIC = 0x53434C53; // 'SQLC'

    // 辅助方法
    void CleanupOldRecords();
    size_t CalculateThroughput(const std::deque<std::pair<std::chrono::steady_clock::time_point, size_t>>& records) const;
};

// 网络异常分类和处理系统 - 异常安全保证
enum NetworkExceptionType {
    CONNECTION_LOST = 0,           // 连接丢失
    CONNECTION_TIMEOUT = 1,        // 连接超时
    AUTHENTICATION_FAILED = 2,     // 认证失败
    PROTOCOL_VIOLATION = 3,        // 协议违规
    RESOURCE_EXHAUSTED = 4,        // 资源耗尽
    DATA_CORRUPTION = 5,           // 数据损坏
    RATE_LIMIT_EXCEEDED = 6,       // 速率限制超限
    SYSTEM_OVERLOAD = 7,           // 系统过载
    NETWORK_UNAVAILABLE = 8,       // 网络不可用
    UNKNOWN_ERROR = 9             // 未知错误
};

class NetworkException : public std::runtime_error {
public:
    NetworkException(NetworkExceptionType type, const std::string& message,
                    const std::string& details = "", bool recoverable = true);
    ~NetworkException() override = default;

    NetworkExceptionType GetType() const { return type_; }
    const std::string& GetDetails() const { return details_; }
    bool IsRecoverable() const { return recoverable_; }
    std::string GetFullMessage() const;

private:
    NetworkExceptionType type_;
    std::string details_;
    bool recoverable_;
};

// 网络异常处理器 - 优雅降级和恢复
class NetworkExceptionHandler {
public:
    NetworkExceptionHandler();
    ~NetworkExceptionHandler() = default;

    // 异常处理策略
    enum RecoveryStrategy {
        IMMEDIATE_RETRY = 0,        // 立即重试
        DELAYED_RETRY = 1,          // 延迟重试
        GRACEFUL_DEGRADATION = 2,   // 优雅降级
        CIRCUIT_BREAKER = 3,        // 断路器
        SYSTEM_SHUTDOWN = 4         // 系统关闭
    };

    // 处理异常
    RecoveryStrategy HandleException(const NetworkException& exception,
                                   std::chrono::milliseconds time_since_last_failure);

    // 恢复策略管理
    void SetMaxRetries(NetworkExceptionType type, int max_retries);
    void SetRetryDelay(NetworkExceptionType type, std::chrono::milliseconds delay);
    void SetCircuitBreakerThreshold(NetworkExceptionType type, int threshold);
    void SetCircuitBreakerTimeout(NetworkExceptionType type, std::chrono::milliseconds timeout);

    // 断路器状态查询
    bool IsCircuitBreakerOpen(NetworkExceptionType type) const;
    std::chrono::milliseconds GetRemainingCircuitBreakerTimeout(NetworkExceptionType type) const;

    // 统计信息
    int GetExceptionCount(NetworkExceptionType type) const;
    int GetRecoveryCount(NetworkExceptionType type) const;
    double GetExceptionRate(NetworkExceptionType type, std::chrono::milliseconds window) const;

    // 重置统计
    void ResetStatistics(NetworkExceptionType type);
    void ResetAllStatistics();

private:
    struct ExceptionStats {
        int exception_count = 0;
        int recovery_count = 0;
        std::chrono::steady_clock::time_point last_exception_time;
        std::chrono::steady_clock::time_point circuit_breaker_opened_time;
        bool circuit_breaker_open = false;
        int max_retries = 3;
        std::chrono::milliseconds retry_delay = std::chrono::seconds(1);
        int circuit_breaker_threshold = 5;
        std::chrono::milliseconds circuit_breaker_timeout = std::chrono::minutes(1);
    };

    std::unordered_map<NetworkExceptionType, ExceptionStats> exception_stats_;
    mutable std::mutex stats_mutex_;

    // 辅助方法
    RecoveryStrategy DetermineStrategy(NetworkExceptionType type, const ExceptionStats& stats,
                                     std::chrono::milliseconds time_since_last_failure);
    void UpdateExceptionStats(NetworkExceptionType type, bool recovery_success);
    void OpenCircuitBreaker(NetworkExceptionType type);
    void CloseCircuitBreaker(NetworkExceptionType type);
};

// 网络监控和日志系统 - 日志记录和监控
class NetworkMonitor {
public:
    NetworkMonitor();
    ~NetworkMonitor() = default;

    // 监控级别
    enum MonitorLevel {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        CRITICAL = 4
    };

    // 日志记录
    void LogEvent(MonitorLevel level, const std::string& component,
                 const std::string& event, const std::string& details = "");

    void LogException(const NetworkException& exception, const std::string& context = "");

    void LogPerformance(const std::string& metric, double value,
                       const std::string& unit = "");

    // 监控指标
    void RecordConnectionEstablished();
    void RecordConnectionLost();
    void RecordMessageSent(size_t size);
    void RecordMessageReceived(size_t size);
    void RecordAuthenticationSuccess();
    void RecordAuthenticationFailure();
    void RecordException(NetworkExceptionType type);

    // 统计查询
    int GetActiveConnections() const;
    size_t GetTotalMessagesSent() const;
    size_t GetTotalMessagesReceived() const;
    size_t GetTotalBytesSent() const;
    size_t GetTotalBytesReceived() const;
    double GetUptime() const; // 秒
    double GetMessagesPerSecond() const;
    double GetBytesPerSecond() const;

    // 健康检查
    bool IsSystemHealthy() const;
    std::string GetHealthReport() const;
    std::vector<std::string> GetActiveAlerts() const;

    // 配置
    void SetLogLevel(MonitorLevel level);
    void SetMaxLogEntries(size_t max_entries);
    void EnablePerformanceMonitoring(bool enable);

private:
    struct LogEntry {
        std::chrono::system_clock::time_point timestamp;
        MonitorLevel level;
        std::string component;
        std::string event;
        std::string details;
    };

    MonitorLevel log_level_;
    size_t max_log_entries_;
    bool performance_monitoring_enabled_;
    std::vector<LogEntry> log_entries_;
    std::chrono::steady_clock::time_point start_time_;

    // 统计数据
    mutable std::mutex stats_mutex_;
    int active_connections_;
    size_t total_messages_sent_;
    size_t total_messages_received_;
    size_t total_bytes_sent_;
    size_t total_bytes_received_;
    int authentication_successes_;
    int authentication_failures_;
    std::unordered_map<NetworkExceptionType, int> exception_counts_;
    std::chrono::steady_clock::time_point last_message_time_;

    // 健康检查阈值
    int max_consecutive_failures_ = 10;
    double max_exception_rate_ = 0.1; // 10% per minute
    size_t min_throughput_ = 1000; // 1KB/s minimum

    // 辅助方法
    void CleanupOldLogs();
    bool IsWithinHealthThresholds() const;
    std::string FormatLogEntry(const LogEntry& entry) const;
    void CheckHealthAlerts(std::vector<std::string>& alerts) const;
};

// 网络稳定性保证器 - 系统稳定性保证
class NetworkStabilityGuard {
public:
    NetworkStabilityGuard();
    ~NetworkStabilityGuard() = default;

    // 稳定性策略
    enum StabilityAction {
        NO_ACTION = 0,              // 无操作
        REDUCE_LOAD = 1,           // 减少负载
        THROTTLE_CONNECTIONS = 2,  // 限制连接
        ENABLE_CIRCUIT_BREAKER = 3, // 启用断路器
        GRACEFUL_SHUTDOWN = 4      // 优雅关闭
    };

    // 稳定性评估
    StabilityAction AssessStability(const NetworkMonitor& monitor,
                                  const NetworkExceptionHandler& exception_handler);

    // 负载管理
    void SetMaxConnections(int max_connections);
    void SetMaxThroughput(size_t bytes_per_second);
    void SetMaxExceptionRate(double exceptions_per_minute);

    // 稳定性控制
    bool ShouldAcceptNewConnection() const;
    bool ShouldThrottleRequests() const;
    std::chrono::milliseconds GetRecommendedDelay() const;

    // 自适应调整
    void AdjustParameters(const NetworkMonitor& monitor);
    void ResetToDefaults();

    // 统计信息
    int GetCurrentLoadLevel() const; // 0-100
    std::string GetStabilityReport() const;

private:
    // 配置参数
    int max_connections_;
    size_t max_throughput_;
    double max_exception_rate_;

    // 当前状态
    mutable std::mutex stability_mutex_;
    int current_connections_;
    size_t current_throughput_;
    double current_exception_rate_;
    StabilityAction last_action_;
    std::chrono::steady_clock::time_point last_assessment_time_;

    // 自适应参数
    double load_reduction_factor_ = 0.8;
    int connection_throttle_threshold_ = 80; // 80% of max
    double exception_rate_threshold_ = 0.05; // 5% per minute

    // 辅助方法
    int CalculateLoadLevel(const NetworkMonitor& monitor) const;
    double CalculateExceptionRate(const NetworkExceptionHandler& exception_handler) const;
    StabilityAction DetermineAction(int load_level, double exception_rate, size_t throughput) const;
};

// 服务器网络管理器
class ServerNetworkManager {
public:
    ServerNetworkManager(int port, int max_connections = 100);
    ~ServerNetworkManager();
    
    bool Start();
    void Stop();
    bool IsRunning() const { return running_; }
    void ProcessEvents();
    void SetSqlExecutor(std::shared_ptr<sqlcc::SqlExecutor> sql_executor);

#ifdef __linux__
    void EnableTLS(bool enabled);
    bool ConfigureTLSServer(const std::string& cert_path,
                            const std::string& key_path,
                            const std::string& ca_cert_path = "");
#endif

private:
    void AcceptConnection();
    
    int port_;
    int max_connections_;
    sqlcc::FileDescriptor listen_fd_;
    sqlcc::FileDescriptor epoll_fd_;
    bool running_;
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<sqlcc::SqlExecutor> sql_executor_;
    std::unordered_map<int, std::unique_ptr<ConnectionHandler>> connections_;
#ifdef __linux__
    bool tls_enabled_ = false;
    sqlcc::utils::SSLContext ssl_ctx_; // SSL_CTX RAII包装器
#endif
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_H
