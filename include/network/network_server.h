#pragma once
#include <cstdint>
#include <memory>
#include "utils/file_descriptor.h"

namespace sqlcc {
namespace network {

class NetworkServer {
public:

    // 处理客户端连接（双协议支持）
    void handle_client(sqlcc::FileDescriptor&& client_fd);

    // 新增：MySQL协议认证处理（第一阶段占位符）
    void handle_mysql_authentication(sqlcc::FileDescriptor&& client_fd);

private:
};

} // namespace network
} // namespace sqlcc
