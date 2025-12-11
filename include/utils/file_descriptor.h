#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <memory>
#include <stdexcept>

namespace sqlcc {

/**
 * @class FileDescriptor
 * @brief RAII wrapper for file descriptors with automatic cleanup
 * @details Provides exception-safe management of file descriptors using RAII pattern
 */
class FileDescriptor {
public:
    /**
     * @brief Default constructor
     * @details Creates an invalid file descriptor (-1)
     */
    FileDescriptor() noexcept : fd_(-1) {}

    /**
     * @brief Constructor with file descriptor
     * @param fd The file descriptor to manage
     */
    explicit FileDescriptor(int fd) noexcept : fd_(fd) {}

    /**
     * @brief Destructor
     * @details Automatically closes the file descriptor if valid
     */
    ~FileDescriptor() noexcept {
        close();
    }

    // Disable copy operations
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    /**
     * @brief Move constructor
     */
    FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }

    /**
     * @brief Move assignment operator
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
     * @brief Get the underlying file descriptor
     * @return The file descriptor value
     */
    int get() const noexcept {
        return fd_;
    }

    /**
     * @brief Check if the file descriptor is valid
     * @return true if valid, false otherwise
     */
    bool valid() const noexcept {
        return fd_ >= 0;
    }

    /**
     * @brief Explicit conversion to int
     */
    explicit operator int() const noexcept {
        return fd_;
    }

    /**
     * @brief Reset the file descriptor
     * @param fd New file descriptor value (-1 to close current)
     */
    void reset(int fd = -1) noexcept {
        close();
        fd_ = fd;
    }

    /**
     * @brief Release ownership of the file descriptor
     * @return The file descriptor (caller takes ownership)
     */
    int release() noexcept {
        int temp = fd_;
        fd_ = -1;
        return temp;
    }

    /**
     * @brief Close the file descriptor if valid
     */
    void close() noexcept {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    /**
     * @brief Create a socket file descriptor
     * @param domain Socket domain (e.g., AF_INET)
     * @param type Socket type (e.g., SOCK_STREAM)
     * @param protocol Protocol (usually 0)
     * @return FileDescriptor instance
     */
    static FileDescriptor create_socket(int domain, int type, int protocol) {
        int fd = ::socket(domain, type, protocol);
        if (fd < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        return FileDescriptor(fd);
    }

    /**
     * @brief Create a TCP socket
     * @return FileDescriptor instance for TCP socket
     */
    static FileDescriptor create_tcp_socket() {
        return create_socket(AF_INET, SOCK_STREAM, 0);
    }

    /**
     * @brief Create a UDP socket
     * @return FileDescriptor instance for UDP socket
     */
    static FileDescriptor create_udp_socket() {
        return create_socket(AF_INET, SOCK_DGRAM, 0);
    }

    /**
     * @brief Create an epoll file descriptor
     * @param flags Epoll flags
     * @return FileDescriptor instance for epoll
     */
    static FileDescriptor create_epoll(int flags = 0) {
        int fd = ::epoll_create1(flags);
        if (fd < 0) {
            throw std::runtime_error("Failed to create epoll");
        }
        return FileDescriptor(fd);
    }

    /**
     * @brief Accept a connection on a listening socket
     * @param sockfd Listening socket file descriptor
     * @param addr Client address structure (can be nullptr)
     * @param addrlen Address length (can be nullptr)
     * @param flags Additional flags
     * @return FileDescriptor instance for the accepted connection
     */
    static FileDescriptor accept(int sockfd, struct sockaddr* addr = nullptr,
                                socklen_t* addrlen = nullptr, int flags = 0) {
        int fd = ::accept4(sockfd, addr, addrlen, flags);
        if (fd < 0) {
            throw std::runtime_error("Failed to accept connection");
        }
        return FileDescriptor(fd);
    }

private:
    int fd_;  ///< File descriptor value
};

} // namespace sqlcc
