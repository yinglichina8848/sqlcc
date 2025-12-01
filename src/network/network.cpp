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
Session::Session(int session_id) : session_id_(session_id), authenticated_(false), encryption_disabled_(false), authentication_disabled_(false), aes_encryptor_(nullptr) {}

void Session::SetEncryptionDisabled(bool disabled) {
    encryption_disabled_ = disabled;
}

bool Session::IsEncryptionDisabled() const {
    return encryption_disabled_;
}

void Session::SetAuthenticationDisabled(bool disabled) {
    authentication_disabled_ = disabled;
}

bool Session::IsAuthenticationDisabled() const {
    return authentication_disabled_;
}

void Session::SetAESEncryptor(std::shared_ptr<class AESEncryptor> encryptor) {
    aes_encryptor_ = encryptor;
}

std::shared_ptr<class AESEncryptor> Session::GetAESEncryptor() const {
    return aes_encryptor_;
}

bool Session::IsAESEncryptionEnabled() const {
    return aes_encryptor_ != nullptr && !encryption_disabled_;
}

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

void ClientConnection::EnableTLS(bool enabled) {
#ifdef __linux__
    tls_enabled_ = enabled;
#else
    (void)enabled;
#endif
}

#ifdef __linux__
bool ClientConnection::ConfigureTLSClient(const std::string& ca_cert_path) {
    ca_cert_path_ = ca_cert_path;
    return true;
}
#endif

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
    
    // 转换IP地址或解析主机名
    if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
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

    // 如果启用TLS，则进行TLS握手
    if (tls_enabled_) {
        SSL_library_init();
        SSL_load_error_strings();
        const SSL_METHOD* method = TLS_client_method();
        ssl_ctx_ = SSL_CTX_new(method);
        if (!ssl_ctx_) {
            Disconnect();
            return false;
        }
        if (!ca_cert_path_.empty()) {
            if (SSL_CTX_load_verify_locations(ssl_ctx_, ca_cert_path_.c_str(), nullptr) != 1) {
                Disconnect();
                return false;
            }
            SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_PEER, nullptr);
        }
        ssl_ = SSL_new(ssl_ctx_);
        SSL_set_fd(ssl_, socket_fd_);
        if (SSL_connect(ssl_) <= 0) {
            Disconnect();
            return false;
        }
    }
#endif
    return true;
}

void ClientConnection::Disconnect() {
#ifdef __linux__
    if (connected_ && socket_fd_ >= 0) {
        if (tls_enabled_ && ssl_) {
            SSL_shutdown(ssl_);
            SSL_free(ssl_);
            ssl_ = nullptr;
        }
        if (tls_enabled_ && ssl_ctx_) {
            SSL_CTX_free(ssl_ctx_);
            ssl_ctx_ = nullptr;
        }
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
    if (tls_enabled_ && ssl_) {
        size_t total_sent = 0;
        while (total_sent < data.size()) {
            int sent = SSL_write(ssl_, data.data() + total_sent, static_cast<int>(data.size() - total_sent));
            if (sent <= 0) {
                return false;
            }
            total_sent += sent;
        }
        return true;
    }
    size_t total_sent = 0;
    while (total_sent < data.size()) {
        ssize_t sent = send(socket_fd_, data.data() + total_sent, data.size() - total_sent, 0);
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
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
    if (tls_enabled_ && ssl_) {
        // 设置超时，避免无限阻塞
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        int bytes = SSL_read(ssl_, buffer.data(), static_cast<int>(buffer.size()));
        if (bytes <= 0) {
            return std::vector<char>();
        }
        buffer.resize(bytes);
        return buffer;
    }
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

void ClientNetworkManager::EnableTLS(bool enabled) {
    connection_->EnableTLS(enabled);
}

#ifdef __linux__
bool ClientNetworkManager::ConfigureTLSClient(const std::string& ca_cert_path) {
    return connection_->ConfigureTLSClient(ca_cert_path);
}
#endif

void ClientNetworkManager::Disconnect() {
    connection_->Disconnect();
}

bool ClientNetworkManager::IsConnected() const {
    return connection_->IsConnected();
}

bool ClientNetworkManager::SendRequest(const std::vector<char>& request) {
    // 在客户端侧，仅对消息体进行加密并追加HMAC
    if (IsAESEncryptionEnabled()) {
        std::vector<char> msg = request;
        MessageHeader* header = reinterpret_cast<MessageHeader*>(msg.data());
        std::vector<char> body(msg.begin() + sizeof(MessageHeader), msg.end());
        auto aes = GetAESEncryptor();
        std::vector<uint8_t> ct = aes->Encrypt(std::vector<uint8_t>(body.begin(), body.end()));
        std::vector<uint8_t> mac = HMACSHA256::Compute(aes->GetKeyBytes(), ct);
        std::vector<char> new_body(ct.begin(), ct.end());
        new_body.insert(new_body.end(), mac.begin(), mac.end());
        header->length = static_cast<uint32_t>(new_body.size());
        msg.resize(sizeof(MessageHeader) + new_body.size());
        std::memcpy(msg.data(), header, sizeof(MessageHeader));
        std::memcpy(msg.data() + sizeof(MessageHeader), new_body.data(), new_body.size());
        return connection_->SendData(msg);
    }
    return connection_->SendData(request);
}

std::vector<char> ClientNetworkManager::ReceiveResponse() {
    auto resp = connection_->ReceiveData();
    if (resp.size() < sizeof(MessageHeader)) return resp;
    MessageHeader* header = reinterpret_cast<MessageHeader*>(resp.data());
    if (IsAESEncryptionEnabled() && header->length >= 32) {
        const char* body_ptr = resp.data() + sizeof(MessageHeader);
        std::vector<uint8_t> ciphertext(body_ptr, body_ptr + header->length - 32);
        std::vector<uint8_t> mac(body_ptr + header->length - 32, body_ptr + header->length);
        auto aes = GetAESEncryptor();
        if (!HMACSHA256::Verify(aes->GetKeyBytes(), ciphertext, mac)) {
            return resp; // 返回原始响应以便上层处理错误
        }
        std::vector<uint8_t> plaintext = aes->Decrypt(ciphertext);
        MessageHeader new_header = *header;
        new_header.length = static_cast<uint32_t>(plaintext.size());
        std::vector<char> out(sizeof(MessageHeader) + plaintext.size());
        std::memcpy(out.data(), &new_header, sizeof(MessageHeader));
        std::memcpy(out.data() + sizeof(MessageHeader), plaintext.data(), plaintext.size());
        return out;
    }
    return resp;
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

bool ClientNetworkManager::InitiateKeyExchange() {
    // 发送密钥交换请求
    MessageHeader header;
    header.magic = 0x53514C43; // 'SQLC'
    header.length = 0;
    header.type = KEY_EXCHANGE;
    header.flags = 0;
    header.sequence_id = 2;
    
    std::vector<char> message(sizeof(MessageHeader));
    std::memcpy(message.data(), &header, sizeof(MessageHeader));
    
    std::cout << "Sending KEY_EXCHANGE request" << std::endl;
    if (!SendRequest(message)) {
        std::cerr << "Failed to send KEY_EXCHANGE" << std::endl;
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 接收密钥交换响应
    std::vector<char> response = ReceiveResponse();
    std::cout << "KEY_EXCHANGE response size: " << response.size() << std::endl;
    if (response.size() < sizeof(MessageHeader)) {
        std::cerr << "Invalid KEY_EXCHANGE response size" << std::endl;
        return false;
    }
    
    MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(response.data());
    
    // 检查是否是密钥交换确认
    if (resp_header->type != KEY_EXCHANGE_ACK) {
        std::cerr << "Unexpected response type: " << resp_header->type << " (expected " << KEY_EXCHANGE_ACK << ")" << std::endl;
        return false;
    }
    
    // 提取IV（密钥交换确认消息的体部）
    if (resp_header->length < 16) {
        std::cerr << "Invalid IV length: " << resp_header->length << std::endl;
        return false;
    }
    
    const char* iv_data = response.data() + sizeof(MessageHeader);
    std::vector<uint8_t> iv(iv_data, iv_data + resp_header->length);
    
    try {
        // 带有客户端生成的残余串阮盆密钥
        auto encryption_key = std::make_shared<network::EncryptionKey>(
            network::EncryptionKey::GenerateRandom(32, 16)->GetKey(),  // 生成残余串阮盆密钥
            iv  // 使用服务器发送的IV
        );
        
        aes_encryptor_ = std::make_shared<network::AESEncryptor>(encryption_key);
        std::cout << "KEY_EXCHANGE successful" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize AES encryptor: " << e.what() << std::endl;
        return false;
    }
}

void ClientNetworkManager::SetAESEncryptor(std::shared_ptr<class AESEncryptor> encryptor) {
    aes_encryptor_ = encryptor;
}

std::shared_ptr<class AESEncryptor> ClientNetworkManager::GetAESEncryptor() const {
    return aes_encryptor_;
}

bool ClientNetworkManager::IsAESEncryptionEnabled() const {
    return aes_encryptor_ != nullptr;
}

std::vector<char> ClientNetworkManager::EncryptMessage(const std::vector<char>& message) {
    if (!aes_encryptor_) {
        return message;
    }
    try {
        std::vector<uint8_t> data(message.begin(), message.end());
        std::vector<uint8_t> ciphertext = aes_encryptor_->Encrypt(data);
        // 计算HMAC-SHA256并追加
        std::vector<uint8_t> mac = HMACSHA256::Compute(aes_encryptor_->GetKeyBytes(), ciphertext);
        std::vector<char> out(ciphertext.begin(), ciphertext.end());
        out.insert(out.end(), mac.begin(), mac.end());
        return out;
    } catch (const std::exception& e) {
        std::cerr << "Encryption failed: " << e.what() << std::endl;
        return message;
    }
}

std::vector<char> ClientNetworkManager::DecryptMessage(const std::vector<char>& message) {
    if (!aes_encryptor_) {
        return message;
    }
    try {
        if (message.size() < 32) {
            return message;
        }
        // 分离MAC
        std::vector<uint8_t> mac(message.end() - 32, message.end());
        std::vector<uint8_t> ciphertext(message.begin(), message.end() - 32);
        // 验证HMAC
        if (!HMACSHA256::Verify(aes_encryptor_->GetKeyBytes(), ciphertext, mac)) {
            std::cerr << "HMAC verification failed" << std::endl;
            return message;
        }
        std::vector<uint8_t> plaintext = aes_encryptor_->Decrypt(ciphertext);
        return std::vector<char>(plaintext.begin(), plaintext.end());
    } catch (const std::exception& e) {
        std::cerr << "Decryption failed: " << e.what() << std::endl;
        return message;
    }
}


// ConnectionHandler实现
ConnectionHandler::ConnectionHandler(int fd, std::shared_ptr<SessionManager> session_manager, std::shared_ptr<sqlcc::SqlExecutor> sql_executor)
    : fd_(fd), session_manager_(std::move(session_manager)), sql_executor_(std::move(sql_executor)), 
      session_(nullptr), closed_(false)
#ifdef __linux__
      , ssl_(nullptr), tls_enabled_(false)
#endif
{
#ifdef __linux__
    // 设置为非阻塞模式
    int flags = fcntl(fd_, F_GETFL, 0);
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
#endif
}

ConnectionHandler::~ConnectionHandler() {
#ifdef __linux__
    if (ssl_) { SSL_free(ssl_); ssl_ = nullptr; }
#endif
}

void ConnectionHandler::SetTLS(struct ssl_st* ssl, bool enabled) {
    ssl_ = ssl;
    tls_enabled_ = enabled;
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
    ssize_t bytes_read = 0;
    if (tls_enabled_ && ssl_) {
        bytes_read = SSL_read(ssl_, buffer.data(), static_cast<int>(buffer.size()));
    } else {
        bytes_read = recv(fd_, buffer.data(), buffer.size(), 0);
    }
    
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
        ssize_t bytes_sent = 0;
        if (tls_enabled_ && ssl_) {
            bytes_sent = SSL_write(ssl_, data.data(), static_cast<int>(data.size()));
        } else {
            bytes_sent = send(fd_, data.data(), data.size(), 0);
        }
        
        if (bytes_sent > 0) {
            write_queue_.pop();
        } else if (bytes_sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 注册 EPOLLOUT 事件以在可写时继续发送
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                ev.data.ptr = this;
                // 在 ServerNetworkManager 中需要 epoll_fd，这里暂时不修改，等待后续优化
                break;
            } else {
                Close();
                break;
            }
        }
    }
#endif
}

void ConnectionHandler::SendMessage(const std::vector<char>& message) {
#ifdef __linux__
    std::vector<char> to_send = message;
    // 如果AES已启用，则仅对消息体进行加密并追加HMAC（除 KEY_EXCHANGE_ACK 外）
    MessageHeader* msg_header = reinterpret_cast<MessageHeader*>(to_send.data());
    if (session_ && session_->IsAESEncryptionEnabled() && msg_header->type != KEY_EXCHANGE_ACK) {
        MessageHeader* header = reinterpret_cast<MessageHeader*>(to_send.data());
        std::vector<char> body(to_send.begin() + sizeof(MessageHeader), to_send.end());
        // 加密体并追加MAC
        auto aes = session_->GetAESEncryptor();
        std::vector<uint8_t> ct = aes->Encrypt(std::vector<uint8_t>(body.begin(), body.end()));
        std::vector<uint8_t> mac = HMACSHA256::Compute(aes->GetKeyBytes(), ct);
        std::vector<char> new_body(ct.begin(), ct.end());
        new_body.insert(new_body.end(), mac.begin(), mac.end());
        header->length = static_cast<uint32_t>(new_body.size());
        to_send.resize(sizeof(MessageHeader) + new_body.size());
        std::memcpy(to_send.data(), header, sizeof(MessageHeader));
        std::memcpy(to_send.data() + sizeof(MessageHeader), new_body.data(), new_body.size());
    }

    std::lock_guard<std::mutex> lock(write_mutex_);
    bool queue_was_empty = write_queue_.empty();
    write_queue_.push(to_send);
    
    // 如果队列之前为空，尝试立即发送，否则等待 EPOLLOUT 事件
    if (queue_was_empty) {
        // 释放锁后调用 HandleWrite 避免死锁
        write_mutex_.unlock();
        HandleWrite();
        write_mutex_.lock();
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
    
    if (header->magic != 0x53514C43) {
        return;
    }
    
    // 若启用AES，则尝试将消息体解密（除密钥交换外）
    std::vector<char> working = data;
    if (session_ && session_->IsAESEncryptionEnabled() && header->type != KEY_EXCHANGE && header->length >= 32) {
        const char* body_ptr = working.data() + sizeof(MessageHeader);
        std::vector<uint8_t> ciphertext(body_ptr, body_ptr + header->length - 32);
        std::vector<uint8_t> mac(body_ptr + header->length - 32, body_ptr + header->length);
        auto aes = session_->GetAESEncryptor();
        if (!HMACSHA256::Verify(aes->GetKeyBytes(), ciphertext, mac)) {
            SendErrorMessage("HMAC verification failed");
            return;
        }
        std::vector<uint8_t> plaintext = aes->Decrypt(ciphertext);
        // 重建消息，将明文作为新体
        MessageHeader new_header = *header;
        new_header.length = static_cast<uint32_t>(plaintext.size());
        working.resize(sizeof(MessageHeader) + plaintext.size());
        std::memcpy(working.data(), &new_header, sizeof(MessageHeader));
        std::memcpy(working.data() + sizeof(MessageHeader), plaintext.data(), plaintext.size());
        header = reinterpret_cast<MessageHeader*>(working.data());
    }
    
    // 根据消息类型处理
    switch (header->type) {
        case CONNECT:
            HandleConnectMessage(working);
            break;
        case AUTH:
            HandleAuthMessage(working);
            break;
        case QUERY:
            HandleQueryMessage(working);
            break;
        case KEY_EXCHANGE:
            HandleKeyExchangeMessage(working);
            break;
        default:
            break;
    }
}

void ConnectionHandler::HandleConnectMessage(const std::vector<char>& data) {
    // 创建会话
    session_ = session_manager_->CreateSession();
    
    // 检查客户端连接消息中的标志
    uint32_t client_flags = 0;
    if (data.size() >= sizeof(MessageHeader)) {
        MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
        client_flags = header->flags;
        
        // 如果客户端请求禁用加密，记录到会话中
        if (client_flags & 0x01) {
            session_->SetEncryptionDisabled(true);
        }
        
        // 如果客户端请求禁用认证，记录到会话中
        if (client_flags & 0x02) {
            session_->SetAuthenticationDisabled(true);
            // 自动通过认证
            session_->SetAuthenticated("anonymous");
        }
    }
    
    // 发送连接确认消息，包含相同的标志
    MessageHeader ack_header;
    ack_header.magic = 0x53514C43; // 'SQLC'
    ack_header.length = 0;
    ack_header.type = CONN_ACK;
    ack_header.flags = client_flags; // 回显客户端的标志
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
    if (!session_) {
        // 会话不存在
        SendErrorMessage("Session not found");
        return;
    }
    
    // 检查是否需要认证（只有在未禁用认证的情况下才要求认证）
    if (!session_->IsAuthenticationDisabled() && !session_->IsAuthenticated()) {
        // 未禁用认证但用户未认证，拒绝请求
        SendErrorMessage("Not authenticated");
        return;
    }

    MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
    
    // 确保有足够的数据
    if (data.size() < sizeof(MessageHeader) + header->length) {
        SendErrorMessage("Invalid query message");
        return;
    }
    
    // 获取查询语句
    std::string query(data.data() + sizeof(MessageHeader), header->length);
    
    // 执行SQL查询（在最小化构建下，直接回显查询）
    std::string result;
    bool success = true;
    result = std::string("ECHO: ") + query;
    
    // 构造查询结果消息
    MessageHeader result_header;
    result_header.magic = 0x53514C43; // 'SQLC'
    result_header.length = result.length();
    result_header.type = QUERY_RESULT;
    result_header.flags = success ? 0 : 1; // 使用flags表示执行结果
    result_header.sequence_id = header->sequence_id;

    std::vector<char> result_msg(sizeof(MessageHeader) + result.length());
    std::memcpy(result_msg.data(), &result_header, sizeof(MessageHeader));
    std::memcpy(result_msg.data() + sizeof(MessageHeader), result.c_str(), result.length());
    SendMessage(result_msg);
}

void ConnectionHandler::HandleKeyExchangeMessage(const std::vector<char>& data) {
    // 处理密钥交换消息
    MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
    
    // 检查是否有session
    if (!session_) {
        SendErrorMessage("Session not found");
        return;
    }
    
    try {
        // 生成AES-256密钥和IV
        auto encryption_key = network::EncryptionKey::GenerateRandom(32, 16); // AES-256 = 32字节
        auto aes_encryptor = std::make_shared<network::AESEncryptor>(encryption_key);
        
        // 将AES加密器设置到session中
        session_->SetAESEncryptor(aes_encryptor);
        
        // 发送密钥交换确认消息，含有了IV
        std::string ack_data(reinterpret_cast<const char*>(encryption_key->GetIV().data()), 
                            encryption_key->GetIV().size());
        
        MessageHeader ack_header;
        ack_header.magic = 0x53514C43; // 'SQLC'
        ack_header.length = ack_data.length();
        ack_header.type = KEY_EXCHANGE_ACK;
        ack_header.flags = 0x01; // 使用flag表示已含有AES加密
        ack_header.sequence_id = header->sequence_id;

        std::vector<char> ack_msg(sizeof(MessageHeader) + ack_data.length());
        std::memcpy(ack_msg.data(), &ack_header, sizeof(MessageHeader));
        std::memcpy(ack_msg.data() + sizeof(MessageHeader), ack_data.c_str(), ack_data.length());
        SendMessage(ack_msg);
    } catch (const std::exception& e) {
        SendErrorMessage(std::string("Key exchange failed: ") + e.what());
    }
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

std::vector<char> ConnectionHandler::EncryptMessage(const std::vector<char>& message) {
    if (!session_ || !session_->IsAESEncryptionEnabled()) {
        return message;
    }
    try {
        auto aes_encryptor = session_->GetAESEncryptor();
        if (!aes_encryptor) {
            return message;
        }
        std::vector<uint8_t> data(message.begin(), message.end());
        std::vector<uint8_t> ciphertext = aes_encryptor->Encrypt(data);
        std::vector<uint8_t> mac = HMACSHA256::Compute(aes_encryptor->GetKeyBytes(), ciphertext);
        std::vector<char> out(ciphertext.begin(), ciphertext.end());
        out.insert(out.end(), mac.begin(), mac.end());
        return out;
    } catch (const std::exception& e) {
        std::cerr << "Encryption failed: " << e.what() << std::endl;
        return message;
    }
}

std::vector<char> ConnectionHandler::DecryptMessage(const std::vector<char>& message) {
    if (!session_ || !session_->IsAESEncryptionEnabled()) {
        return message;
    }
    try {
        auto aes_encryptor = session_->GetAESEncryptor();
        if (!aes_encryptor) {
            return message;
        }
        if (message.size() < 32) {
            return message;
        }
        std::vector<uint8_t> mac(message.end() - 32, message.end());
        std::vector<uint8_t> ciphertext(message.begin(), message.end() - 32);
        if (!HMACSHA256::Verify(aes_encryptor->GetKeyBytes(), ciphertext, mac)) {
            std::cerr << "HMAC verification failed" << std::endl;
            return message;
        }
        std::vector<uint8_t> plaintext = aes_encryptor->Decrypt(ciphertext);
        return std::vector<char>(plaintext.begin(), plaintext.end());
    } catch (const std::exception& e) {
        std::cerr << "Decryption failed: " << e.what() << std::endl;
        return message;
    }
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

void ServerNetworkManager::EnableTLS(bool enabled) {
#ifdef __linux__
    tls_enabled_ = enabled;
#else
    (void)enabled;
#endif
}

bool ServerNetworkManager::ConfigureTLSServer(const std::string& cert_path,
                                              const std::string& key_path,
                                              const std::string& ca_cert_path) {
#ifdef __linux__
    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD* method = TLS_server_method();
    ssl_ctx_ = SSL_CTX_new(method);
    if (!ssl_ctx_) return false;
    if (SSL_CTX_use_certificate_file(ssl_ctx_, cert_path.c_str(), SSL_FILETYPE_PEM) != 1) return false;
    if (SSL_CTX_use_PrivateKey_file(ssl_ctx_, key_path.c_str(), SSL_FILETYPE_PEM) != 1) return false;
    // 服务端不强制校验客户端证书，避免握手失败
    if (!ca_cert_path.empty()) {
        // 可选地加载CA以支持链验证，但不设置SSL_VERIFY_PEER
        SSL_CTX_load_verify_locations(ssl_ctx_, ca_cert_path.c_str(), nullptr);
    }
    return true;
#else
    (void)cert_path; (void)key_path; (void)ca_cert_path; return false;
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

    // 创建连接处理器，传入SQL执行器
    ConnectionHandler* handler = new ConnectionHandler(client_fd, session_manager_, sql_executor_);

    // 若启用TLS，在该连接上进行握手
    if (tls_enabled_ && ssl_ctx_) {
        SSL* ssl = SSL_new(ssl_ctx_);
        SSL_set_fd(ssl, client_fd);
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags & ~O_NONBLOCK);
        int ret = SSL_accept(ssl);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        if (ret <= 0) {
            SSL_free(ssl);
            delete handler;
            close(client_fd);
            return;
        }
        handler->SetTLS(ssl, true);
    }
    
    // 添加到epoll（水平触发以简化处理）
    struct epoll_event ev;
    ev.events = EPOLLIN; // 水平触发
    ev.data.ptr = handler;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        if (tls_enabled_ && handler) {
#ifdef __linux__
            if (handler) {
                // 释放SSL
            }
#endif
        }
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