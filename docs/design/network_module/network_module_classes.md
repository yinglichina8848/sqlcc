# Network Module 类设计文档

## 类列表

### NetworkStabilityGuard

**定义位置**: `include/network/network_stability_guard.h`

**定义**:
```cpp
class NetworkStabilityGuard {
public:
    NetworkStabilityGuard();
    ~NetworkStabilityGuard() = default;

    // 稳定性策略
    enum StabilityAction {
        NO_ACTION = 0,              // 无操作
        R...
```

**构造函数**:
- `NetworkStabilityGuard`
- `AssessStability`
- `SetMaxConnections`
- `SetMaxThroughput`
- `SetMaxExceptionRate`
- `AdjustParameters`
- `ResetToDefaults`

**公有方法**:
- `稳定性评估
    StabilityAction AssessStability`
- `负载管理
    void SetMaxConnections`
- `void SetMaxThroughput`
- `void SetMaxExceptionRate`
- `自适应调整
    void AdjustParameters`
- `void ResetToDefaults`

---

### DataTransmissionValidator

**定义位置**: `include/network/data_transmission_validator.h`

**定义**:
```cpp
class DataTransmissionValidator {
public:
    DataTransmissionValidator();
    ~DataTransmissionValidator() = default;

    // 消息头验证
    bool ValidateMessageHeader(const MessageHeader& header) const;
...
```

**构造函数**:
- `DataTransmissionValidator`
- `FragmentMessage`
- `ReassembleFragments`
- `CanAcceptMessage`
- `RecordMessageSent`
- `RecordMessageReceived`
- `SetMaxMessageSize`
- `SetMaxBufferSize`
- `SetFragmentSize`
- `SetRateLimit`
- `CleanupOldRecords`

**公有方法**:
- `流量控制
    bool CanAcceptMessage`
- `void RecordMessageSent`
- `void RecordMessageReceived`
- `配置参数
    void SetMaxMessageSize`
- `void SetMaxBufferSize`
- `void SetFragmentSize`
- `void SetRateLimit`
- `辅助方法
    void CleanupOldRecords`

---

### NetworkMonitor

**定义位置**: `include/network/network_monitor.h`

**定义**:
```cpp
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
...
```

**构造函数**:
- `NetworkMonitor`
- `LogEvent`
- `LogException`
- `LogPerformance`
- `RecordConnectionEstablished`
- `RecordConnectionLost`
- `RecordMessageSent`
- `RecordMessageReceived`
- `RecordAuthenticationSuccess`
- `RecordAuthenticationFailure`
- `RecordException`
- `SetLogLevel`
- `SetMaxLogEntries`
- `EnablePerformanceMonitoring`
- `CleanupOldLogs`

**公有方法**:
- `日志记录
    void LogEvent`
- `void LogException`
- `void LogPerformance`
- `监控指标
    void RecordConnectionEstablished`
- `void RecordConnectionLost`
- `void RecordMessageSent`
- `void RecordMessageReceived`
- `void RecordAuthenticationSuccess`
- `void RecordAuthenticationFailure`
- `void RecordException`
- `配置
    void SetLogLevel`
- `void SetMaxLogEntries`
- `void EnablePerformanceMonitoring`
- `辅助方法
    void CleanupOldLogs`

---

### MySQLProtocolHandler

**定义位置**: `include/network/mysql_protocol.h`

**定义**:
```cpp
class MySQLProtocolHandler {
public:
    explicit MySQLProtocolHandler(sqlcc::FileDescriptor&& client_fd);

    // 发送握手包
    void send_handshake();

    // 处理客户端响应
    bool handle_client_response();

...
```

**构造函数**:
- `MySQLProtocolHandler`
- `send_handshake`
- `handle_client_response`
- `send_auth_success`
- `send_auth_error`
- `send_query_result`
- `send_packet`
- `receive_packet`
- `send_error_packet`
- `encode_length_encoded_string`
- `encode_length`

**公有方法**:
- `explicit MySQLProtocolHandler`
- `发送握手包
    void send_handshake`
- `处理客户端响应
    bool handle_client_response`
- `发送认证成功响应
    bool send_auth_success`
- `发送认证失败响应
    bool send_auth_error`
- `发送查询结果
    bool send_query_result`
- `bool send_packet`
- `发送错误包
    bool send_error_packet`
- `长度编码辅助函数
    void encode_length_encoded_string`
- `void encode_length`

---

### Session

**定义位置**: `include/network/session.h`

**定义**:
```cpp
class Session {
public:
    /**
     * @brief 构造函数
     * @param session_id 会话ID，必须为正数
     */
    explicit Session(int session_id);

    /**
     * @brief 设置加密禁用状态
     * @param disabled true表示禁用加密，f...
```

**构造函数**:
- `Session`
- `SetEncryptionDisabled`
- `SetAuthenticationDisabled`
- `SetAESEncryptor`
- `SetAuthenticated`

**公有方法**:
- `explicit Session`
- `void SetEncryptionDisabled`
- `void SetAuthenticationDisabled`
- `void SetAESEncryptor`
- `void SetAuthenticated`

---

### DatabaseManager

**定义位置**: `include/network/multi_threaded_network_manager.h`

**定义**:
```cpp

class DatabaseManager;
class SqlExecutor;

/**
 * @brief 网络请求上下文
 */
struct NetworkRequest {
  int client_id;
  std::string request_data;
  std::chrono::steady_clock::time_point received_at;
  std::s...
```

---

### KeyRotationPolicy

**定义位置**: `include/network/key_rotation_policy.h`

**定义**:
```cpp
class KeyRotationPolicy {
public:
    explicit KeyRotationPolicy(size_t interval_messages = 1000)
        : interval_(interval_messages) {}
    bool ShouldRotate(size_t messages_sent) const { return i...
```

---

### ConnectionHandler

**定义位置**: `include/network/connection_handler.h`

**定义**:
```cpp
class ConnectionHandler {
public:
    /**
     * @brief 构造函数
     * @param fd 文件描述符
     * @param session_manager 会话管理器
     * @param sql_executor SQL执行器
     * @param user_manager 用户管理器
     */
    C...
```

**构造函数**:
- `ConnectionHandler`
- `ConnectionHandler`
- `SetTLS`
- `SetAESEncryptor`
- `HandleEvent`
- `SendMessage`
- `EncryptMessage`
- `DecryptMessage`
- `HandleRead`
- `HandleWrite`
- `ProcessMessage`
- `HandleConnectMessage`
- `HandleAuthMessage`
- `HandleQueryMessage`
- `HandleKeyExchangeMessage`
- `SendErrorMessage`
- `TrySendImmediately`
- `Close`
- `AnalyzeQueryOperation`
- `ExtractDatabaseFromQuery`
- `ExtractTableFromQuery`

**析构函数**:
- `ConnectionHandler`

**公有方法**:
- `void SetTLS`
- `void SetAESEncryptor`
- `void HandleEvent`
- `void SendMessage`
- `void HandleRead`
- `void HandleWrite`
- `void ProcessMessage`
- `void HandleConnectMessage`
- `void HandleAuthMessage`
- `void HandleQueryMessage`
- `void HandleKeyExchangeMessage`
- `void SendErrorMessage`
- `bool TrySendImmediately`
- `void Close`
- `PermissionOperation AnalyzeQueryOperation`
- `string ExtractDatabaseFromQuery`
- `string ExtractTableFromQuery`

---

### MessageProcessor

**定义位置**: `include/network/message_processor.h`

**定义**:
```cpp
class MessageProcessor {
public:
    MessageProcessor(std::shared_ptr<SessionManager> session_manager);

private:
    std::shared_ptr<SessionManager> session_manager_;
};
```

**构造函数**:
- `MessageProcessor`

---

### ClientNetworkManager

**定义位置**: `include/network/client_network_manager.h`

**定义**:
```cpp
class ClientNetworkManager {
public:
    /**
     * @brief 构造函数
     * @param host 服务器主机地址
     * @param port 服务器端口号
     */
    ClientNetworkManager(const std::string& host, int port);

    /**
     ...
```

**构造函数**:
- `ClientNetworkManager`
- `ClientNetworkManager`
- `Connect`
- `EnableTLS`
- `ConfigureTLSClient`
- `Disconnect`
- `SendRequest`
- `ReceiveResponse`
- `SendAuthMessage`
- `InitiateKeyExchange`
- `SetAESEncryptor`
- `EncryptMessage`
- `DecryptMessage`
- `GenerateSequenceId`

**析构函数**:
- `ClientNetworkManager`

**公有方法**:
- `bool Connect`
- `void EnableTLS`
- `__linux__
    bool ConfigureTLSClient`
- `void Disconnect`
- `bool SendRequest`
- `bool SendAuthMessage`
- `bool InitiateKeyExchange`
- `void SetAESEncryptor`
- `uint32_t GenerateSequenceId`

---

### NetworkExceptionHandler

**定义位置**: `include/network/network_exception_handler.h`

**定义**:
```cpp
class NetworkExceptionHandler {
public:
    NetworkExceptionHandler();
    ~NetworkExceptionHandler() = default;

    // 异常处理策略
    enum RecoveryStrategy {
        IMMEDIATE_RETRY = 0,        // 立即重试
...
```

**构造函数**:
- `NetworkExceptionHandler`
- `HandleException`
- `SetMaxRetries`
- `SetRetryDelay`
- `SetCircuitBreakerThreshold`
- `SetCircuitBreakerTimeout`
- `ResetStatistics`
- `ResetAllStatistics`
- `seconds`
- `minutes`
- `DetermineStrategy`
- `UpdateExceptionStats`
- `OpenCircuitBreaker`
- `CloseCircuitBreaker`

**公有方法**:
- `处理异常
    RecoveryStrategy HandleException`
- `恢复策略管理
    void SetMaxRetries`
- `void SetRetryDelay`
- `void SetCircuitBreakerThreshold`
- `void SetCircuitBreakerTimeout`
- `重置统计
    void ResetStatistics`
- `void ResetAllStatistics`
- `辅助方法
    RecoveryStrategy DetermineStrategy`
- `void UpdateExceptionStats`
- `void OpenCircuitBreaker`
- `void CloseCircuitBreaker`

---

### NetworkException

**定义位置**: `include/network/network_exception.h`

**定义**:
```cpp
class NetworkException : public std::runtime_error {
public:
    /**
     * @brief 构造函数
     * @param type 异常类型
     * @param message 错误消息
     * @param details 详细信息
     * @param recoverable 是否可恢复
  ...
```

**构造函数**:
- `NetworkException`

---

### NetworkServer

**定义位置**: `include/network/network_server.h`

**定义**:
```cpp

class NetworkServer {
public:

    // 处理客户端连接（双协议支持）
    void handle_client(sqlcc::FileDescriptor&& client_fd);

    // 新增：MySQL协议认证处理（第一阶段占位符）
    void handle_mysql_authentication(sqlcc::FileDescrip...
```

**构造函数**:
- `handle_client`
- `handle_mysql_authentication`

**公有方法**:
- `void handle_client`
- `void handle_mysql_authentication`

---

### ServerNetworkManager

**定义位置**: `include/network/server_network_manager.h`

**定义**:
```cpp
class ServerNetworkManager {
public:
    /**
     * @brief 构造函数
     * @param port 监听端口
     * @param max_connections 最大连接数
     * @param thread_pool_size 线程池大小
     */
    ServerNetworkManager(int po...
```

**构造函数**:
- `ServerNetworkManager`
- `ServerNetworkManager`
- `Start`
- `Stop`
- `ProcessEvents`
- `SetSqlExecutor`
- `EnableTLS`
- `ConfigureTLSServer`
- `AcceptConnection`

**析构函数**:
- `ServerNetworkManager`

**公有方法**:
- `bool Start`
- `void Stop`
- `void ProcessEvents`
- `void SetSqlExecutor`
- `void EnableTLS`
- `__linux__
    bool ConfigureTLSServer`
- `void AcceptConnection`

---

### ClientConnection

**定义位置**: `include/network/client_connection.h`

**定义**:
```cpp
class ClientConnection {
public:
    /**
     * @brief 构造函数
     * @param host 服务器主机地址
     * @param port 服务器端口号
     */
    ClientConnection(const std::string& host, int port);

    /**
     * @brief...
```

**构造函数**:
- `ClientConnection`
- `ClientConnection`
- `EnableTLS`
- `ConfigureTLSClient`
- `Connect`
- `Disconnect`
- `SendData`
- `ReceiveData`

**析构函数**:
- `ClientConnection`

**公有方法**:
- `void EnableTLS`
- `__linux__
    bool ConfigureTLSClient`
- `bool Connect`
- `void Disconnect`
- `bool SendData`

---

### ConnectionStateMachine

**定义位置**: `include/network/connection_state_machine.h`

**定义**:
```cpp
class ConnectionStateMachine {
public:
    /**
     * @brief 构造函数
     */
    ConnectionStateMachine();

    /**
     * @brief 析构函数
     */
    ~ConnectionStateMachine() = default;

    /**
     * @br...
```

**构造函数**:
- `ConnectionStateMachine`
- `TransitionTo`
- `Reset`
- `ForceError`
- `SetStateChangeCallback`
- `SetTimeouts`
- `StartConnectionTimer`
- `StopConnectionTimer`
- `ResetConnectionTimer`
- `EnableAutoReconnect`
- `IncrementRetryCount`
- `ResetRetryCount`
- `EnableKeepAlive`
- `UpdateLastActivity`

**公有方法**:
- `bool TransitionTo`
- `void Reset`
- `void ForceError`
- `void SetStateChangeCallback`
- `void SetTimeouts`
- `void StartConnectionTimer`
- `void StopConnectionTimer`
- `void ResetConnectionTimer`
- `void EnableAutoReconnect`
- `void IncrementRetryCount`
- `void ResetRetryCount`
- `void EnableKeepAlive`
- `void UpdateLastActivity`

---

### MessageSerializer

**定义位置**: `include/network/message_serializer.h`

**定义**:
```cpp
class MessageSerializer {
public:
    /**
     * @brief 构造函数
     */
    MessageSerializer();

    /**
     * @brief 析构函数
     */
    ~MessageSerializer();

    /**
     * @brief 序列化消息
     * @param t...
```

**构造函数**:
- `MessageSerializer`
- `MessageSerializer`
- `Serialize`
- `DeserializeHeader`
- `DeserializeMessage`
- `CalculateCRC32`
- `VerifyCRC32`
- `主机字节序转网络字节序`
- `网络字节序转主机字节序`
- `InitializeCRC32Table`

**析构函数**:
- `MessageSerializer`

**公有方法**:
- `bool DeserializeHeader`
- `bool DeserializeMessage`
- `static uint32_t CalculateCRC32`
- `static bool VerifyCRC32`
- `brief 主机字节序转网络字节序`
- `brief 网络字节序转主机字节序`
- `static void InitializeCRC32Table`

---

### SessionManager

**定义位置**: `include/network/session_manager.h`

**定义**:
```cpp
class SessionManager {
public:
    /**
     * @brief 构造函数
     */
    SessionManager();

    /**
     * @brief 创建新会话
     * @return 新创建的会话智能指针
     */
    std::shared_ptr<Session> CreateSession();

  ...
```

**构造函数**:
- `SessionManager`
- `CreateSession`
- `GetSession`
- `DestroySession`
- `Authenticate`
- `CheckPermission`

**公有方法**:
- `void DestroySession`
- `bool Authenticate`
- `bool CheckPermission`

---

### AESEncryptor

**定义位置**: `include/network/encryption/aes_encryptor.h`

**定义**:
```cpp
class AESEncryptor {
public:
    /**
     * @brief 构造函数
     * @param encryption_key 加密密钥和IV
     */
    explicit AESEncryptor(std::shared_ptr<EncryptionKey> encryption_key);

    /**
     * @brief 析构...
```

**构造函数**:
- `AESEncryptor`
- `AESEncryptor`
- `UpdateKey`
- `IsAvailable`
- `GetKeyBytes`
- `InitializeContext`

**析构函数**:
- `AESEncryptor`

**公有方法**:
- `explicit AESEncryptor`
- `void UpdateKey`
- `static bool IsAvailable`
- `bool InitializeContext`

---

### EncryptionKey

**定义位置**: `include/network/encryption/encryption_key.h`

**定义**:
```cpp
class EncryptionKey {
public:
    /**
     * @brief 构造函数
     * @param key 加密密钥
     * @param iv 初始化向量
     */
    EncryptionKey(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

    ...
```

**构造函数**:
- `EncryptionKey`
- `GenerateRandom`

---

### PBKDF2

**定义位置**: `include/network/encryption/pbkdf2.h`

**定义**:
```cpp
class PBKDF2 {
public:
    /**
     * @brief 从口令派生密钥
     * @param passphrase 用户口令
     * @param salt 盐值
     * @param iterations 迭代次数（推荐10000或更多）
     * @param key_len 派生密钥长度（字节）
     * @return 派生的密钥...
```

**构造函数**:
- `Derive`

---

### SimpleEncryptor

**定义位置**: `include/network/encryption/simple_encryptor.h`

**定义**:
```cpp
class SimpleEncryptor {
public:
    /**
     * @brief 构造函数
     * @param key 加密密钥字符串
     */
    explicit SimpleEncryptor(const std::string& key);

    /**
     * @brief 加密数据
     * @param data 待加密的数据...
```

**构造函数**:
- `SimpleEncryptor`

**公有方法**:
- `explicit SimpleEncryptor`

---

### HMACSHA256

**定义位置**: `include/network/encryption/hmac_sha256.h`

**定义**:
```cpp
class HMACSHA256 {
public:
    /**
     * @brief 计算HMAC-SHA256值
     * @param key 密钥
     * @param data 待认证的数据
     * @return HMAC-SHA256值
     */
    static std::vector<uint8_t> Compute(const std::ve...
```

**构造函数**:
- `Compute`
- `Verify`

**公有方法**:
- `static bool Verify`

---

