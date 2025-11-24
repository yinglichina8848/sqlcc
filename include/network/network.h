/**
 * @file network.h
 * @brief 网络通信模块头文件
 * 
 * 该文件定义了SQLCC数据库系统的网络通信相关类和结构体
 */

#ifndef SQLCC_NETWORK_H
#define SQLCC_NETWORK_H

#include <string>
#include <memory>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>

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

private:
    int session_id_;
    bool authenticated_;
    std::string user_;
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

// 客户端连接类
class ClientConnection {
public:
    ClientConnection(const std::string& host, int port);
    ~ClientConnection();
    
    bool Connect();
    void Disconnect();
    bool IsConnected() const;
    bool SendData(const std::vector<char>& data);
    std::vector<char> ReceiveData();

private:
    std::string host_;
    int port_;
    bool connected_;
    int socket_fd_;
};

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

private:
    std::unique_ptr<ClientConnection> connection_;
    std::shared_ptr<SessionManager> session_manager_;
};

// 连接处理器
class ConnectionHandler {
public:
    ConnectionHandler(int fd, std::shared_ptr<SessionManager> session_manager);
    ~ConnectionHandler();
    
    int GetFd() const;
    bool IsClosed() const;
    void HandleEvent(uint32_t events);
    void ProcessMessage(const std::vector<char>& data);

private:
    void HandleRead();
    void HandleWrite();
    void SendMessage(const std::vector<char>& message);
    void Close();
    
    void HandleConnectMessage(const std::vector<char>& data);
    void HandleAuthMessage(const std::vector<char>& data);
    void HandleQueryMessage(const std::vector<char>& data);
    void HandleKeyExchangeMessage(const std::vector<char>& data);
    void SendErrorMessage(const std::string& error);
    
    int fd_;
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<Session> session_;
    bool closed_;
    std::queue<std::vector<char>> write_queue_;
    std::mutex write_mutex_;
};

// 消息处理器
class MessageProcessor {
public:
    MessageProcessor(std::shared_ptr<SessionManager> session_manager);
    
private:
    std::shared_ptr<SessionManager> session_manager_;
};

// 服务器网络管理器
class ServerNetworkManager {
public:
    ServerNetworkManager(int port, int max_connections = 100);
    ~ServerNetworkManager();
    
    bool Start();
    void Stop();
    void ProcessEvents();

private:
    void AcceptConnection();
    
    int port_;
    int max_connections_;
    int listen_fd_;
    int epoll_fd_;
    bool running_;
    std::shared_ptr<SessionManager> session_manager_;
    std::unordered_map<int, ConnectionHandler*> connections_;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_H