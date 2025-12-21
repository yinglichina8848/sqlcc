#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <string>

namespace sqlcc {
namespace network {

class SessionManager;
class SqlExecutor;

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
     */
    ConnectionHandler(sqlcc::FileDescriptor&& fd,
                     std::shared_ptr<SessionManager> session_manager,
                     std::shared_ptr<sqlcc::SqlExecutor> sql_executor);

    /**
     * @brief 析构函数
     */
    ~ConnectionHandler();

    /**
     * @brief 设置TLS连接
     * @param ssl SSL连接对象
     * @param enabled 是否启用TLS
     */
    void SetTLS(struct ssl_st* ssl, bool enabled);

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

    sqlcc::FileDescriptor fd_;                              ///< 文件描述符
    std::shared_ptr<SessionManager> session_manager_;       ///< 会话管理器
    std::shared_ptr<sqlcc::SqlExecutor> sql_executor_;      ///< SQL执行器
    std::shared_ptr<Session> session_;                      ///< 当前会话
    bool closed_;                                           ///< 连接关闭标志
    std::queue<std::vector<char>> write_queue_;            ///< 写队列
    std::mutex write_mutex_;                                ///< 写队列互斥锁

    // TLS相关成员
#ifdef __linux__
    struct ssl_st* ssl_;                                    ///< SSL连接对象
    bool tls_enabled_;                                      ///< TLS启用标志
#endif
};

// 消息头结构定义
#pragma pack(push, 1)
struct MessageHeader {
    uint32_t magic;         ///< 魔数 (0x53514C43 = 'SQLC')
    uint32_t length;        ///< 消息体长度
    uint8_t type;           ///< 消息类型
    uint8_t flags;          ///< 标志位
    uint32_t sequence_id;   ///< 序列号
};
#pragma pack(pop)

// 消息类型定义
enum MessageType {
    CONNECT = 1,            ///< 连接请求
    CONN_ACK = 2,           ///< 连接确认
    AUTH = 3,               ///< 认证请求
    AUTH_ACK = 4,           ///< 认证确认
    QUERY = 5,              ///< 查询请求
    QUERY_RESULT = 6,       ///< 查询结果
    KEY_EXCHANGE = 7,       ///< 密钥交换请求
    KEY_EXCHANGE_ACK = 8,   ///< 密钥交换确认
    ERROR = 9               ///< 错误消息
};

} // namespace network
} // namespace sqlcc
