/**
 * @file file_descriptor.h
 * @brief 文件描述符RAII封装类
 * @details 提供安全的文件描述符管理，防止资源泄漏
 * @author AI助手
 * @date 2025-12-11
 */

#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <system_error>
#include <utility>

// For socket operations
#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#endif

namespace sqlcc {
namespace utils {

/**
 * @class FileDescriptor
 * @brief 文件描述符RAII封装类
 * @details 使用RAII模式管理文件描述符，确保自动关闭和资源安全
 */
class FileDescriptor {
public:
    /**
     * @brief 默认构造函数
     * @details 创建一个无效的文件描述符
     */
    FileDescriptor() noexcept : fd_(-1) {}

    /**
     * @brief 构造函数
     * @param fd 文件描述符
     * @details 接管传入的文件描述符的管理权
     */
    explicit FileDescriptor(int fd) noexcept : fd_(fd) {}

    /**
     * @brief 析构函数
     * @details 自动关闭文件描述符
     */
    ~FileDescriptor() noexcept {
        close();
    }

    // 禁止拷贝
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    /**
     * @brief 移动构造函数
     * @param other 要移动的文件描述符对象
     */
    FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }

    /**
     * @brief 移动赋值操作符
     * @param other 要移动的文件描述符对象
     * @return 引用自身
     */
    FileDescriptor& operator=(FileDescriptor&& other) noexcept {
        if (this != &other) {
            close();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    /**
     * @brief 获取文件描述符
     * @return 文件描述符值
     */
    int get() const noexcept { return fd_; }

    /**
     * @brief 检查文件描述符是否有效
     * @return 是否有效
     */
    bool valid() const noexcept { return fd_ >= 0; }

    /**
     * @brief 显式转换为int
     * @return 文件描述符值
     */
    explicit operator int() const noexcept { return fd_; }

    /**
     * @brief 释放文件描述符
     * @details 关闭文件描述符并将内部状态设为无效
     */
    void reset(int fd = -1) noexcept {
        close();
        fd_ = fd;
    }

    /**
     * @brief 释放所有权
     * @return 文件描述符值
     * @details 返回文件描述符并放弃管理权，调用者负责关闭
     */
    int release() noexcept {
        int temp = fd_;
        fd_ = -1;
        return temp;
    }

#ifdef __linux__
    /**
     * @brief 接受连接
     * @param sockfd 监听套接字文件描述符
     * @param addr 客户端地址结构
     * @param addrlen 地址结构长度
     * @param flags 标志位
     * @return FileDescriptor对象
     * @details 封装accept4系统调用，返回RAII管理的文件描述符
     */
    static FileDescriptor accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags = 0) {
        int fd = ::accept4(sockfd, addr, addrlen, flags);
        return FileDescriptor(fd);
    }

    /**
     * @brief 创建套接字
     * @param domain 协议族
     * @param type 套接字类型
     * @param protocol 协议
     * @return FileDescriptor对象
     * @details 封装socket系统调用，返回RAII管理的文件描述符
     */
    static FileDescriptor create_socket(int domain, int type, int protocol) {
        int fd = ::socket(domain, type, protocol);
        return FileDescriptor(fd);
    }

    /**
     * @brief 创建epoll实例
     * @param flags 标志位
     * @return FileDescriptor对象
     * @details 封装epoll_create1系统调用，返回RAII管理的文件描述符
     */
    static FileDescriptor create_epoll(int flags = 0) {
        int fd = ::epoll_create1(flags);
        return FileDescriptor(fd);
    }
#endif

private:
    /**
     * @brief 关闭文件描述符
     */
    void close() noexcept {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    int fd_;  ///< 文件描述符
};

/**
 * @class SocketDescriptor
 * @brief 套接字描述符RAII封装类
 * @details 专门用于套接字的文件描述符管理
 */
class SocketDescriptor : public FileDescriptor {
public:
    using FileDescriptor::FileDescriptor;

    /**
     * @brief 创建TCP套接字
     * @return SocketDescriptor对象
     */
    static SocketDescriptor create_tcp() {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        return SocketDescriptor(fd);
    }

    /**
     * @brief 创建UDP套接字
     * @return SocketDescriptor对象
     */
    static SocketDescriptor create_udp() {
        int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        return SocketDescriptor(fd);
    }
};

} // namespace utils
} // namespace sqlcc