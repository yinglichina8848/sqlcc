#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <string>

#include "utils/file_descriptor.h"
#include "sql_executor.h"
#include "message_types.h"
#include "core/permission_validator.h"
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