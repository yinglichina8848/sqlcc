#include "network/network.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <queue>
#include <mutex>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <sys/epoll.h>
#include <netdb.h>
#endif

#include "network/encryption.h"

namespace sqlcc {
namespace network {

// Session实现
Session::Session(int session_id) : session_id_(session_id), authenticated_(false) {}

// SessionManager实现
SessionManager::SessionManager() : next_session_id_(1) {}

std::shared_ptr<Session> SessionManager::CreateSession() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    int session_id = next_session_id_++;
    auto session = std::make_shared<Session>(session_id);
    sessions_[session_id] = session;
    return session;
}

std::shared_ptr<Session> SessionManager::GetSession(int session_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
        return it->second.lock();
    }
    return nullptr;
}

void SessionManager::DestroySession(int session_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.erase(session_id);
}

bool SessionManager::Authenticate(int session_id, const std::string& username, 
                                const std::string& password) {
    // 简单的身份验证逻辑
    if (username == "admin" && password == "password") {
        auto session = GetSession(session_id);
        if (session) {
            session->SetAuthenticated(username);
            return true;
        }
    }
    return false;
}

bool SessionManager::CheckPermission(int session_id, const std::string& database,
                                   const std::string& operation) {
    auto session = GetSession(session_id);
    if (!session || !session->IsAuthenticated()) {
        return false;
    }
    // 简单的权限检查逻辑
    return true;
}

// ClientConnection实现
ClientConnection::ClientConnection(const std::string& host, int port)
    : host_(host), port_(port), connected_(false), socket_fd_(-1) {}

ClientConnection::~ClientConnection() {
    Disconnect();
}

bool ClientConnection::Connect() {
#ifdef __linux__
    // 创建socket
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        return false;
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    
    // 转换IP地址
    if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
        // 如果不是有效的IP地址，尝试解析主机名
        struct hostent* he = gethostbyname(host_.c_str());
        if (he == nullptr) {
            close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }
        std::memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);
    }

    // 连接到服务器
    if (connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    connected_ = true;
#endif
    return true;
}

void ClientConnection::Disconnect() {
#ifdef __linux__
    if (connected_ && socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
        connected_ = false;
    }
#endif
}

bool ClientConnection::IsConnected() const {
    return connected_;
}

bool ClientConnection::SendData(const std::vector<char>& data) {
#ifdef __linux__
    if (!connected_ || socket_fd_ < 0) {
        return false;
    }

    size_t total_sent = 0;
    while (total_sent < data.size()) {
        ssize_t sent = send(socket_fd_, data.data() + total_sent, data.size() - total_sent, 0);
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 非阻塞模式下缓冲区满，短暂等待后重试
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            return false;
        }
        total_sent += sent;
    }
#endif
    return true;
}

std::vector<char> ClientConnection::ReceiveData() {
#ifdef __linux__
    std::vector<char> buffer(4096);
    ssize_t received = recv(socket_fd_, buffer.data(), buffer.size(), 0);
    
    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 非阻塞模式下没有数据可读
            return std::vector<char>();
        }
        return std::vector<char>(); // 错误发生
    } else if (received == 0) {
        // 连接被对方关闭
        connected_ = false;
        return std::vector<char>();
    }
    
    buffer.resize(received);
    return buffer;
#else
    return std::vector<char>();
#endif
}

// ClientNetworkManager实现
ClientNetworkManager::ClientNetworkManager(const std::string& host, int port)
    : connection_(std::make_unique<ClientConnection>(host, port)),
      session_manager_(std::make_shared<SessionManager>()) {}

ClientNetworkManager::~ClientNetworkManager() = default;

bool ClientNetworkManager::Connect() {
    return connection_->Connect();
}

void ClientNetworkManager::Disconnect() {
    connection_->Disconnect();
}

bool ClientNetworkManager::IsConnected() const {
    return connection_->IsConnected();
}

bool ClientNetworkManager::SendRequest(const std::vector<char>& request) {
    return connection_->SendData(request);
}

std::vector<char> ClientNetworkManager::ReceiveResponse() {
    return connection_->ReceiveData();
}

bool ClientNetworkManager::SendAuthMessage(const std::string& username, const std::string& password) {
    // 构造认证消息
    // 格式: [uint32_t username_len][uint32_t password_len][username][password]
    uint32_t username_len = static_cast<uint32_t>(username.length());
    uint32_t password_len = static_cast<uint32_t>(password.length());
    
    size_t body_size = 2 * sizeof(uint32_t) + username_len + password_len;
    std::vector<char> message(sizeof(MessageHeader) + body_size);
    
    // 填充消息头
    MessageHeader* header = reinterpret_cast<MessageHeader*>(message.data());
    header->magic = 0x53514C43; // 'SQLC'
    header->length = static_cast<uint32_t>(body_size);
    header->type = AUTH;
    header->flags = 0;
    header->sequence_id = 1;
    
    // 填充消息体
    char* body = message.data() + sizeof(MessageHeader);
    *reinterpret_cast<uint32_t*>(body) = username_len;
    *reinterpret_cast<uint32_t*>(body + sizeof(uint32_t)) = password_len;
    std::memcpy(body + 2 * sizeof(uint32_t), username.c_str(), username_len);
    std::memcpy(body + 2 * sizeof(uint32_t) + username_len, password.c_str(), password_len);
    
    return SendRequest(message);
}


// ConnectionHandler实现
ConnectionHandler::ConnectionHandler(int fd, std::shared_ptr<SessionManager> session_manager)
    : fd_(fd), session_manager_(std::move(session_manager)), closed_(false) {
#ifdef __linux__
    // 设置为非阻塞模式
    int flags = fcntl(fd_, F_GETFL, 0);
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
#endif
}

ConnectionHandler::~ConnectionHandler() {
    Close();
}

int ConnectionHandler::GetFd() const {
    return fd_;
}

bool ConnectionHandler::IsClosed() const {
    return closed_;
}

void ConnectionHandler::HandleEvent(uint32_t events) {
#ifdef __linux__
    if (events & EPOLLIN) {
        HandleRead();
    }
    if (events & EPOLLOUT) {
        HandleWrite();
    }
    if (events & (EPOLLERR | EPOLLHUP)) {
        Close();
    }
#endif
}

void ConnectionHandler::HandleRead() {
#ifdef __linux__
    // 读取数据并处理
    std::vector<char> buffer(4096);
    ssize_t bytes_read = recv(fd_, buffer.data(), buffer.size(), 0);
    
    if (bytes_read > 0) {
        buffer.resize(bytes_read);
        ProcessMessage(buffer);
    } else if (bytes_read == 0) {
        // 客户端关闭连接
        Close();
    } else {
        // 错误发生
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            Close();
        }
    }
#endif
}

void ConnectionHandler::HandleWrite() {
#ifdef __linux__
    // 处理写事件（如果有待发送的数据）
    std::lock_guard<std::mutex> lock(write_mutex_);
    while (!write_queue_.empty()) {
        const std::vector<char>& data = write_queue_.front();
        ssize_t bytes_sent = send(fd_, data.data(), data.size(), 0);
        
        if (bytes_sent > 0) {
            // 成功发送数据
            write_queue_.pop();
        } else if (bytes_sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 发送缓冲区满，稍后重试
                break;
            } else {
                // 错误发生，关闭连接
                Close();
                break;
            }
        }
    }
#endif
}

void ConnectionHandler::SendMessage(const std::vector<char>& message) {
#ifdef __linux__
    std::lock_guard<std::mutex> lock(write_mutex_);
    write_queue_.push(message);
    
    // 尝试立即发送
    if (write_queue_.size() == 1) {
        HandleWrite();
    }
#endif
}

void ConnectionHandler::Close() {
    if (!closed_) {
        closed_ = true;
#ifdef __linux__
        close(fd_);
#endif
    }
}

void ConnectionHandler::ProcessMessage(const std::vector<char>& data) {
    // 处理接收到的消息
    if (data.size() < sizeof(MessageHeader)) {
        return;
    }

    MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
    
    // 检查消息魔数
    if (header->magic != 0x53514C43) { // 'SQLC'
        return;
    }
    
    // 根据消息类型处理
    switch (header->type) {
        case CONNECT:
            HandleConnectMessage(data);
            break;
        case AUTH:
            HandleAuthMessage(data);
            break;
        case QUERY:
            HandleQueryMessage(data);
            break;
        case KEY_EXCHANGE:
            HandleKeyExchangeMessage(data);
            break;
        default:
            // 未知消息类型
            break;
    }
}

void ConnectionHandler::HandleConnectMessage(const std::vector<char>& data) {
    // 创建会话
    session_ = session_manager_->CreateSession();
    
    // 发送连接确认消息
    MessageHeader ack_header;
    ack_header.magic = 0x53514C43; // 'SQLC'
    ack_header.length = 0;
    ack_header.type = CONN_ACK;
    ack_header.flags = 0;
    ack_header.sequence_id = 1;

    std::vector<char> ack_msg(sizeof(MessageHeader));
    std::memcpy(ack_msg.data(), &ack_header, sizeof(MessageHeader));
    SendMessage(ack_msg);
}

void ConnectionHandler::HandleAuthMessage(const std::vector<char>& data) {
    if (data.size() < sizeof(MessageHeader)) {
        return;
    }

    MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
    if (data.size() < sizeof(MessageHeader) + header->length) {
        return;
    }

    // 解析新的认证数据格式
    // 格式: [uint32_t username_len][uint32_t password_len][username][password]
    const char* body = data.data() + sizeof(MessageHeader);
    if (header->length < 2 * sizeof(uint32_t)) {
        return;
    }

    uint32_t username_len = *reinterpret_cast<const uint32_t*>(body);
    uint32_t password_len = *reinterpret_cast<const uint32_t*>(body + sizeof(uint32_t));
    
    if (header->length != 2 * sizeof(uint32_t) + username_len + password_len) {
        return;
    }

    std::string username(body + 2 * sizeof(uint32_t), username_len);
    std::string password(body + 2 * sizeof(uint32_t) + username_len, password_len);

    bool authenticated = session_manager_->Authenticate(session_->GetSessionId(), username, password);
    
    // 发送认证确认消息
    MessageHeader ack_header;
    ack_header.magic = 0x53514C43; // 'SQLC'
    ack_header.length = 0;
    ack_header.type = AUTH_ACK;
    ack_header.flags = authenticated ? 0 : 1; // 使用flags表示认证结果
    ack_header.sequence_id = header->sequence_id;

    std::vector<char> ack_msg(sizeof(MessageHeader));
    std::memcpy(ack_msg.data(), &ack_header, sizeof(MessageHeader));
    SendMessage(ack_msg);
}

void ConnectionHandler::HandleQueryMessage(const std::vector<char>& data) {
    if (!session_ || !session_->IsAuthenticated()) {
        // 未认证的会话不能执行查询
        SendErrorMessage("Not authenticated");
        return;
    }

    MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
    
    // 确保有足够的数据
    if (data.size() < sizeof(MessageHeader) + header->length) {
        SendErrorMessage("Invalid query message");
        return;
    }
    
    // 构造查询结果消息
    std::string query(data.data() + sizeof(MessageHeader), header->length);
    std::string result = "Executed query: " + query;
    
    MessageHeader result_header;
    result_header.magic = 0x53514C43; // 'SQLC'
    result_header.length = result.length();
    result_header.type = QUERY_RESULT;
    result_header.flags = 0;
    result_header.sequence_id = header->sequence_id;

    std::vector<char> result_msg(sizeof(MessageHeader) + result.length());
    std::memcpy(result_msg.data(), &result_header, sizeof(MessageHeader));
    std::memcpy(result_msg.data() + sizeof(MessageHeader), result.c_str(), result.length());
    SendMessage(result_msg);
}

void ConnectionHandler::HandleKeyExchangeMessage(const std::vector<char>& data) {
    // 处理密钥交换消息
    MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
    
    // 构造密钥交换确认消息
    std::string ack_data = "Key exchange completed";
    
    MessageHeader ack_header;
    ack_header.magic = 0x53514C43; // 'SQLC'
    ack_header.length = ack_data.length();
    ack_header.type = KEY_EXCHANGE_ACK;
    ack_header.flags = 0;
    ack_header.sequence_id = header->sequence_id;

    std::vector<char> ack_msg(sizeof(MessageHeader) + ack_data.length());
    std::memcpy(ack_msg.data(), &ack_header, sizeof(MessageHeader));
    std::memcpy(ack_msg.data() + sizeof(MessageHeader), ack_data.c_str(), ack_data.length());
    SendMessage(ack_msg);
}

void ConnectionHandler::SendErrorMessage(const std::string& error) {
    MessageHeader error_header;
    error_header.magic = 0x53514C43; // 'SQLC'
    error_header.length = error.length();
    error_header.type = ERROR;
    error_header.flags = 0;
    error_header.sequence_id = 0;

    std::vector<char> error_msg(sizeof(MessageHeader) + error.length());
    std::memcpy(error_msg.data(), &error_header, sizeof(MessageHeader));
    std::memcpy(error_msg.data() + sizeof(MessageHeader), error.c_str(), error.length());
    SendMessage(error_msg);
}

// MessageProcessor实现
MessageProcessor::MessageProcessor(std::shared_ptr<SessionManager> session_manager)
    : session_manager_(std::move(session_manager)) {}

// ServerNetworkManager实现
ServerNetworkManager::ServerNetworkManager(int port, int max_connections)
    : port_(port), max_connections_(max_connections), listen_fd_(-1), epoll_fd_(-1), running_(false),
      session_manager_(std::make_shared<SessionManager>()) {}

ServerNetworkManager::~ServerNetworkManager() {
    Stop();
}

bool ServerNetworkManager::Start() {
#ifdef __linux__
    // 创建监听socket
    listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_fd_ < 0) {
        return false;
    }

    // 设置socket选项
    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定地址
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    // 开始监听
    if (listen(listen_fd_, SOMAXCONN) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    // 创建epoll实例
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    // 添加监听socket到epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) < 0) {
        close(epoll_fd_);
        close(listen_fd_);
        epoll_fd_ = -1;
        listen_fd_ = -1;
        return false;
    }

    running_ = true;
    return true;
#else
    return false; // 非Linux平台不支持
#endif
}

void ServerNetworkManager::Stop() {
    running_ = false;
    
#ifdef __linux__
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
    
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
#endif
}

void ServerNetworkManager::ProcessEvents() {
#ifdef __linux__
    if (!running_ || epoll_fd_ < 0) {
        return;
    }

    struct epoll_event events[64];
    int nfds = epoll_wait(epoll_fd_, events, 64, 0);
    
    for (int i = 0; i < nfds; i++) {
        if (events[i].data.ptr == nullptr) {
            // 监听socket有事件，接受新连接
            AcceptConnection();
        } else {
            // 客户端连接有事件
            ConnectionHandler* handler = static_cast<ConnectionHandler*>(events[i].data.ptr);
            handler->HandleEvent(events[i].events);
            
            if (handler->IsClosed()) {
                // 从epoll中移除并删除连接处理器
                epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, handler->GetFd(), nullptr);
                connections_.erase(handler->GetFd());
                delete handler;
            }
        }
    }
#endif
}

void ServerNetworkManager::AcceptConnection() {
#ifdef __linux__
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept4(listen_fd_, (struct sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);
    if (client_fd < 0) {
        return;
    }

    // 创建连接处理器
    ConnectionHandler* handler = new ConnectionHandler(client_fd, session_manager_);
    
    // 添加到epoll
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // 边缘触发
    ev.data.ptr = handler;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        delete handler;
        close(client_fd);
        return;
    }

    // 添加到连接映射
    connections_[client_fd] = handler;
#endif
}

} // namespace network
} // namespace sqlcc