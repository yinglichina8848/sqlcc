#pragma once
#include <cstdint>

class NetworkServer {
public:
    
    // 处理客户端连接（双协议支持）
    void handle_client(int client_fd);
    
    // 新增：MySQL协议认证处理（第一阶段占位符）
    void handle_mysql_authentication(int client_fd);
    
private:
};