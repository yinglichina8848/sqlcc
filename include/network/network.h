/**
 * @file network.h
 * @brief 网络通信模块头文件
 * 
 * 该文件定义了SQLCC数据库系统的网络通信模块相关类和接口，
 * 包括客户端和服务器端的网络通信功能。
 */

#ifndef SQLCC_NETWORK_H
#define SQLCC_NETWORK_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <atomic>
#include <chrono>

namespace sqlcc {
namespace network {

// 消息头结构
#pragma pack(push, 1)
struct MessageHeader {
    uint32_t magic;        // 魔数: 0x53514C43 ('SQLC')
    uint32_t length;       // 消息体长度
    uint16_t type;         // 消息类型
    uint16_t flags;        // 标志位
    uint64_t sequence_id;  // 序列号
};
#pragma pack(pop)

// 消息类型定义
enum MessageType {
    CONNECT = 0x01,      // 连接请求
    CONN_ACK = 0x02,     // 连接确认
    AUTH = 0x03,         // 认证请求
    AUTH_ACK = 0x04,     // 认证响应
    QUERY = 0x05,        // SQL查询请求
    QUERY_RESULT = 0x06, // 查询结果
    ERROR = 0x07,        // 错误信息
    CLOSE = 0x08,        // 关闭连接请求
    CLOSE_ACK = 0x09,    // 关闭连接确认
    KEY_EXCHANGE = 0x0A, // 密钥交换
    KEY_EXCHANGE_ACK = 0x0B // 密钥交换确认
};

// 消息标志位定义
enum MessageFlags {
    ENCRYPTED = 0x01     // 消息已加密
};

// 前向声明
class Session;
class SessionManager;
class ConnectionHandler;

/**
 * @brief 会话类
 * 
 * 管理客户端会话状态，包括认证信息和会话数据
 */
class Session {
public:
    explicit Session(int session_id);
    
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

/**
 * @brief 会话管理器类
 * 
 * 管理所有客户端会话，包括创建、销毁、认证和权限检查
 */
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

/**
 * @brief 客户端连接类
 * 
 * 处理客户端与服务器之间的具体网络I/O操作
 */
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

/**
 * @brief 客户端网络管理器类
 * 
 * 客户端网络模块的核心类，负责建立与服务器的连接并管理网络通信
 */
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

/**
 * @brief 连接处理器类
 * 
 * 负责处理单个客户端连接的所有网络I/O操作
 */
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

/**
 * @brief 消息处理器类
 * 
 * 负责解析协议消息并调用相应的数据库核心功能
 */
class MessageProcessor {
public:
    explicit MessageProcessor(std::shared_ptr<SessionManager> session_manager);
    ~MessageProcessor();
    
    std::vector<char> ProcessConnectMessage(const std::vector<char>& message);
    std::vector<char> ProcessAuthMessage(const std::vector<char>& message);
    std::vector<char> ProcessQueryMessage(const std::vector<char>& message);
    std::vector<char> ProcessCloseMessage(const std::vector<char>& message);
    
private:
    std::vector<char> BuildResponse(uint16_t type, const std::vector<char>& data);
    
    std::shared_ptr<SessionManager> session_manager_;
};

/**
 * @brief 服务器网络管理器类
 * 
 * 服务器网络模块的核心类，负责监听客户端连接、管理连接池、分发连接处理任务
 */
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

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_H