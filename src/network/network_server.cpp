#include "network/network_server.h"
#include "network/mysql_protocol.h"
#include "../sql_executor.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>

namespace sqlcc {
namespace network {

void NetworkServer::handle_client(sqlcc::FileDescriptor&& client_fd) {
  // 创建SQL执行器实例
  auto sql_executor = std::make_shared<SqlExecutor>();

  // 协议检测：检查是否为MySQL客户端（期望第一个字节为0x0A）
  uint8_t first_byte;
  ssize_t ret = recv(client_fd.get(), &first_byte, 1, MSG_PEEK);
  if (ret == 1 && first_byte == 0x0A) {
    // MySQL协议处理流程
    MySQLProtocolHandler handler(std::move(client_fd));
    handler.send_handshake();

    // 处理客户端登录请求
    if (handler.handle_client_response()) {
      // 进入认证阶段（后续阶段实现）
      // 注意：client_fd已经被移动，这里不能再使用
      // handle_mysql_authentication(std::move(client_fd));
    }
  } else {
    // 使用自定义协议处理器
    // 创建连接处理器来处理客户端请求
    auto session_manager = std::make_shared<SessionManager>();
    auto handler = std::make_unique<ConnectionHandler>(
        std::move(client_fd), session_manager, sql_executor);
    
    // 简化的处理循环 - 实际实现中应使用epoll等机制
    while (!handler->IsClosed()) {
      handler->HandleRead();
      // 添加一个小延迟避免CPU占用过高
      usleep(10000); // 10ms
    }
  }

  // 注意：FileDescriptor会自动管理生命周期，无需手动close
}

// 新增方法声明（需在头文件中添加）
void NetworkServer::handle_mysql_authentication(sqlcc::FileDescriptor&& client_fd) {
  // TODO: 实现caching_sha2_password认证流程
  // 当前阶段仅作为占位符
  (void)client_fd; // 避免未使用参数警告
}

} // namespace network
} // namespace sqlcc