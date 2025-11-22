# SQLCC 网络通信核心模块设计

## 1. 概述

本文档详细描述了 SQLCC 数据库管理系统中网络通信核心模块的设计，包括服务器端网络模块和客户端网络模块的具体实现方案。

## 2. 服务器端网络模块设计

### 2.1 模块结构

服务器端网络模块由以下几个核心类组成：

1. **ServerNetworkManager** - 服务器网络管理器
2. **ConnectionHandler** - 连接处理器
3. **MessageProcessor** - 消息处理器
4. **SessionManager** - 会话管理器

### 2.2 ServerNetworkManager（服务器网络管理器）

#### 2.2.1 功能描述
ServerNetworkManager 是服务器网络模块的核心类，负责监听客户端连接、管理连接池、分发连接处理任务。

#### 2.2.2 主要职责
- 初始化网络监听套接字
- 使用 epoll/ioctl 等异步 I/O 模型处理并发连接
- 管理连接处理器池
- 分发客户端连接到连接处理器
- 处理服务器关闭和资源清理

#### 2.2.3 接口设计
```cpp
class ServerNetworkManager {
public:
    ServerNetworkManager(int port, int max_connections);
    ~ServerNetworkManager();
    
    bool Start();
    void Stop();
    void ProcessEvents();
    
private:
    void AcceptConnection();
    void HandleEvent(int fd, uint32_t events);
    
    int listen_fd_;
    int epoll_fd_;
    int port_;
    int max_connections_;
    std::unordered_map<int, std::shared_ptr<ConnectionHandler>> connections_;
    std::unique_ptr<SessionManager> session_manager_;
};
```

### 2.3 ConnectionHandler（连接处理器）

#### 2.3.1 功能描述
ConnectionHandler 负责处理单个客户端连接的所有网络 I/O 操作。

#### 2.3.2 主要职责
- 读取客户端发送的数据
- 发送数据到客户端
- 管理连接状态
- 解析消息头和消息体
- 与消息处理器协作处理请求

#### 2.3.3 接口设计
```cpp
class ConnectionHandler {
public:
    ConnectionHandler(int fd, std::shared_ptr<SessionManager> session_manager);
    ~ConnectionHandler();
    
    bool ReadData();
    bool WriteData();
    bool ProcessMessage();
    void Close();
    
    int GetFd() const { return fd_; }
    bool IsClosed() const { return closed_; }
    
private:
    bool ParseHeader();
    bool ParseBody();
    
    int fd_;
    bool closed_;
    std::shared_ptr<SessionManager> session_manager_;
    std::unique_ptr<MessageProcessor> message_processor_;
    
    // 接收缓冲区
    std::vector<char> recv_buffer_;
    size_t recv_pos_;
    
    // 发送缓冲区
    std::vector<char> send_buffer_;
    size_t send_pos_;
    
    // 当前正在处理的消息
    MessageHeader current_header_;
    std::vector<char> current_body_;
};
```

### 2.4 MessageProcessor（消息处理器）

#### 2.4.1 功能描述
MessageProcessor 负责解析协议消息并调用相应的数据库核心功能。

#### 2.4.2 主要职责
- 解析不同类型的消息
- 调用数据库核心引擎处理请求
- 构造响应消息
- 处理认证和会话相关逻辑

#### 2.4.3 接口设计
```cpp
class MessageProcessor {
public:
    MessageProcessor(std::shared_ptr<SessionManager> session_manager);
    ~MessageProcessor();
    
    std::vector<char> ProcessConnectMessage(const std::vector<char>& message);
    std::vector<char> ProcessAuthMessage(const std::vector<char>& message);
    std::vector<char> ProcessQueryMessage(const std::vector<char>& message);
    std::vector<char> ProcessCloseMessage(const std::vector<char>& message);
    
private:
    std::vector<char> BuildResponse(uint16_t type, const std::vector<char>& data);
    
    std::shared_ptr<SessionManager> session_manager_;
};
```

### 2.5 SessionManager（会话管理器）

#### 2.5.1 功能描述
SessionManager 负责管理所有客户端会话，包括认证、权限和会话状态。

#### 2.5.2 主要职责
- 创建和销毁会话
- 管理会话状态
- 处理用户认证
- 权限检查

#### 2.5.3 接口设计
```cpp
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
    
private:
    int session_id_;
    bool authenticated_;
    std::string user_;
    std::chrono::steady_clock::time_point last_activity_;
};

class SessionManager {
public:
    SessionManager();
    ~SessionManager();
    
    std::shared_ptr<Session> CreateSession();
    std::shared_ptr<Session> GetSession(int session_id);
    void DestroySession(int session_id);
    
    bool Authenticate(int session_id, const std::string& user, const std::string& password);
    bool CheckPermission(int session_id, const std::string& database, const std::string& operation);
    
private:
    std::unordered_map<int, std::shared_ptr<Session>> sessions_;
    std::atomic<int> next_session_id_;
};
```

## 3. 客户端网络模块设计

### 3.1 模块结构

客户端网络模块由以下几个核心类组成：

1. **ClientNetworkManager** - 客户端网络管理器
2. **ClientConnection** - 客户端连接

### 3.2 ClientNetworkManager（客户端网络管理器）

#### 3.2.1 功能描述
ClientNetworkManager 是客户端网络模块的核心类，负责建立与服务器的连接并管理网络通信。

#### 3.2.2 主要职责
- 建立与服务器的TCP连接
- 发送请求消息到服务器
- 接收服务器响应消息
- 处理连接异常和重连

#### 3.2.3 接口设计
```cpp
class ClientNetworkManager {
public:
    ClientNetworkManager(const std::string& host, int port);
    ~ClientNetworkManager();
    
    bool Connect();
    void Disconnect();
    bool SendRequest(const std::vector<char>& request);
    std::vector<char> ReceiveResponse();
    
    bool IsConnected() const { return connected_; }
    
private:
    std::string host_;
    int port_;
    bool connected_;
    std::unique_ptr<ClientConnection> connection_;
};
```

### 3.3 ClientConnection（客户端连接）

#### 3.3.1 功能描述
ClientConnection 负责处理客户端与服务器之间的具体网络 I/O 操作。

#### 3.3.2 主要职责
- 管理TCP套接字
- 发送数据到服务器
- 从服务器接收数据
- 处理连接异常

#### 3.3.3 接口设计
```cpp
class ClientConnection {
public:
    ClientConnection(const std::string& host, int port);
    ~ClientConnection();
    
    bool Connect();
    void Disconnect();
    bool SendData(const std::vector<char>& data);
    std::vector<char> ReceiveData();
    
    bool IsConnected() const { return connected_; }
    
private:
    std::string host_;
    int port_;
    int fd_;
    bool connected_;
};
```

## 4. 协议消息格式实现

### 4.1 消息头结构
```cpp
#pragma pack(push, 1)
struct MessageHeader {
    uint32_t magic;        // 魔数: 0x53514C43 ('SQLC')
    uint32_t length;       // 消息体长度
    uint16_t type;         // 消息类型
    uint16_t flags;        // 标志位
    uint64_t sequence_id;  // 序列号
};
#pragma pack(pop)
```

### 4.2 消息体结构示例

#### 4.2.1 连接消息 (CONNECT)
```cpp
struct ConnectMessage {
    uint32_t client_version;  // 客户端版本
    char reserved[16];        // 保留字段
};
```

#### 4.2.2 认证消息 (AUTH)
```cpp
struct AuthMessage {
    uint8_t username_length;     // 用户名长度
    char username[256];          // 用户名
    uint8_t auth_data_length;    // 认证数据长度
    char auth_data[512];         // 认证数据
};
```

#### 4.2.3 查询消息 (QUERY)
```cpp
struct QueryMessage {
    uint32_t sql_length;    // SQL语句长度
    char sql[4096];         // SQL语句内容
};
```

## 5. 异常处理和错误恢复

### 5.1 网络异常处理
- 连接断开检测和重连机制
- 超时处理
- 消息完整性校验

### 5.2 错误消息格式
```cpp
struct ErrorMessage {
    uint32_t error_code;      // 错误码
    uint16_t message_length;  // 错误消息长度
    char message[1024];       // 错误消息内容
};
```

## 6. 性能优化策略

### 6.1 连接池
实现连接复用以减少连接建立开销

### 6.2 缓冲区管理
使用环形缓冲区和预分配内存减少内存分配开销

### 6.3 异步I/O
采用 epoll/ioctl 等异步 I/O 模型提高并发处理能力

## 7. 安全设计

### 7.1 认证安全
- 使用挑战-响应机制避免明文密码传输
- 支持加盐哈希存储密码

### 7.2 传输安全
- 支持 TLS/SSL 加密传输
- 数据完整性校验

## 8. 测试方案

### 8.1 单元测试
- 网络管理器功能测试
- 连接处理器测试
- 消息处理器测试
- 会话管理器测试

### 8.2 集成测试
- 客户端-服务器完整通信流程测试
- 并发连接处理测试
- 异常处理测试

### 8.3 性能测试
- 连接建立/断开性能测试
- 消息处理吞吐量测试
- 高并发场景测试

## 9. 总结

本文档详细描述了 SQLCC 网络通信核心模块的设计方案，包括服务器端和客户端的网络组件。通过模块化设计和清晰的接口定义，确保了系统的可扩展性和可维护性。异步 I/O 模型和连接池等优化措施保证了系统的高性能，而完善的安全机制确保了通信的安全性。