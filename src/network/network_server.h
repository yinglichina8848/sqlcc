/**
 * @file network_server.h
 * @brief SQLCC网络服务器核心 - 双协议支持与连接分发入口
 *
 * NetworkServer是数据库网络层的处理入口，负责接收原始的客户端连接，
 * 智能识别通信协议（原生协议或MySQL协议），并将连接分发给相应的协议处理器。
 *
 * 📚 配套教材参考：
 * - [第14章：数据库网络协议](../../textbook/《数据库系统原理与开发实践》.md#第十四章数据库网络协议)
 * - [14.1 网络层架构设计](../../textbook/《数据库系统原理与开发实践》.md#141-网络层架构设计)
 * - [14.2 多协议支持策略](../../textbook/《数据库系统原理与开发实践》.md#142-多协议支持策略)
 *
 * WHY层 - 设计意图：
 *   为了提高系统的兼容性和易用性，SQLCC支持多种客户端协议。NetworkServer
 *   需要作为一个统一的入口，透明地处理不同类型的客户端连接，无需用户配置
 *   不同的端口，简化了部署和运维复杂度。
 *
 * WHAT层 - 功能说明：
 *   - 统一入口：处理所有进入的TCP连接
 *   - 协议探测：智能识别客户端使用的通信协议
 *   - 连接分发：将连接移交给专门的协议处理器
 *   - MySQL兼容：支持标准MySQL客户端直接连接
 *   - 原生支持：支持SQLCC原生高性能客户端
 *
 * HOW层 - 实现机制：
 *   - 预读取探测：使用recv(MSG_PEEK)预读取首字节判断协议
 *   - 协议特征码：利用MySQL握手包特征（0x0A）进行识别
 *   - 资源移交：使用std::move转移Socket所有权，零拷贝
 *   - 依赖注入：动态创建SqlExecutor等核心组件
 *
 * 协议识别算法：
 *   1. 接收新连接的Socket文件描述符
 *   2. 非破坏性读取第一个字节（MSG_PEEK）
 *   3. 如果首字节是0x0A，识别为MySQL协议（Login Request Packet）
 *   4. 否则默认为SQLCC原生自定义协议
 *   5. 根据识别结果创建相应的ProtocolHandler
 *
 * 架构位置：
 *   [Client] -> [ServerNetworkManager] -> [NetworkServer] -> [ProtocolHandler]
 *                                              |
 *                                      +-------+-------+
 *                                      |               |
 *                                [MySQLHandler]  [NativeHandler]
 *
 * @author SQLCC技术委员会
 * @version 1.1.0
 * @date 2026-02-02
 */

#pragma once
#include <cstdint>
#include <memory>
#include "utils/file_descriptor.h"

namespace sqlcc {
namespace network {

/**
 * @brief 网络服务器类 - 协议分发器
 *
 * WHY层 - 设计意图：
 *   NetworkServer封装了连接处理的具体逻辑，将网络IO（ServerNetworkManager负责）
 *   与业务逻辑（ProtocolHandler负责）解耦。它充当了"协议网关"的角色。
 *
 * WHAT层 - 核心职责：
 *   - 负责单个连接的生命周期初期管理
 *   - 协调底层网络资源与上层业务组件
 *   - 提供MySQL认证流程的扩展点
 *
 * HOW层 - 使用方式：
 *   通常由ServerNetworkManager在accept()返回新的fd后调用：
 *   NetworkServer server;
 *   server.handle_client(std::move(client_fd));
 */
class NetworkServer {
public:

    /**
     * @brief 处理客户端连接（双协议支持）
     *
     * WHY:
     *   这是连接处理的主入口。必须在一个方法内完成协议探测和分发，
     *   以保证连接处理的原子性和逻辑连贯性。
     *
     * WHAT:
     *   接收一个已连接的Socket，探测其协议类型，并启动相应的处理流程。
     *   如果是MySQL客户端，启动MySQL握手；如果是原生客户端，启动命令循环。
     *
     * HOW:
     *   1. 创建SqlExecutor实例作为执行上下文
     *   2. peek socket首字节
     *   3. IF (byte == 0x0A) -> MySQLProtocolHandler
     *   4. ELSE -> ConnectionHandler (Native)
     *   5. 无论何种情况，fd的所有权都会被转移给具体的处理器
     *
     * @param client_fd 客户端连接的文件描述符（所有权转移）
     */
    void handle_client(sqlcc::FileDescriptor&& client_fd);

    /**
     * @brief MySQL协议认证处理（第一阶段占位符）
     *
     * WHY:
     *   MySQL的认证流程较为复杂（握手 -> 响应 -> 验证 -> 结果），
     *   需要独立的逻辑流来处理caching_sha2_password等机制。
     *
     * WHAT:
     *   处理MySQL协议握手后的认证响应阶段。
     *   目前作为后续完整实现的扩展接口保留。
     *
     * @param client_fd 客户端连接的文件描述符
     */
    void handle_mysql_authentication(sqlcc::FileDescriptor&& client_fd);

private:
};

} // namespace network
} // namespace sqlcc