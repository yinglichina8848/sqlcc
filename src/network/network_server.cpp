#include "network/network_server.h"
#include "network/mysql_protocol.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace sqlcc {
namespace network {

void NetworkServer::handle_client(sqlcc::utils::FileDescriptor& client_fd) {

  // 协议检测：检查是否为MySQL客户端（期望第一个字节为0x0A）
  uint8_t first_byte;
  ssize_t ret = recv(client_fd.get(), &first_byte, 1, MSG_PEEK);
  if (ret == 1 && first_byte == 0x0A) {
    // MySQL协议处理流程
    MySQLProtocolHandler handler(client_fd.get());
    handler.send_handshake();

    // 处理客户端登录请求
    if (handler.handle_client_response()) {
      // 进入认证阶段（后续阶段实现）
      handle_mysql_authentication(client_fd);
    }
  } else {
    // 使用MySQL协议处理器作为默认处理器
    MySQLProtocolHandler handler(client_fd.get());
    handler.send_handshake();

    // 处理客户端响应
    if (handler.handle_client_response()) {
      handle_mysql_authentication(client_fd);
    }
  }

  // 注意：FileDescriptor会自动管理生命周期，无需手动close
}

// 新增方法声明（需在头文件中添加）
void NetworkServer::handle_mysql_authentication(sqlcc::utils::FileDescriptor& client_fd) {
  // TODO: 实现caching_sha2_password认证流程
  // 当前阶段仅作为占位符
  (void)client_fd; // 避免未使用参数警告
}

} // namespace network
} // namespace sqlcc
