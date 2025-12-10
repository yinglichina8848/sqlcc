/**
 * @file ssl_wrapper.h
 * @brief SSL资源RAII包装器
 * 
 * 该文件提供了SSL和SSL_CTX资源的RAII管理类，
 * 确保SSL资源在异常情况下也能正确释放
 */

#ifndef SQLCC_SSL_WRAPPER_H
#define SQLCC_SSL_WRAPPER_H

#include <memory>

#ifdef __linux__
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

namespace sqlcc {
namespace utils {

#ifdef __linux__

// SSL_CTX的RAII包装器
class SSLContext {
public:
    // 构造函数
    SSLContext() : ctx_(nullptr) {}
    
    // 从原始SSL_CTX指针构造
    explicit SSLContext(SSL_CTX* ctx) : ctx_(ctx) {}
    
    // 析构函数，自动释放SSL_CTX
    ~SSLContext() {
        if (ctx_) {
            SSL_CTX_free(ctx_);
        }
    }
    
    // 移动构造函数
    SSLContext(SSLContext&& other) noexcept : ctx_(other.ctx_) {
        other.ctx_ = nullptr;
    }
    
    // 移动赋值操作符
    SSLContext& operator=(SSLContext&& other) noexcept {
        if (this != &other) {
            if (ctx_) {
                SSL_CTX_free(ctx_);
            }
            ctx_ = other.ctx_;
            other.ctx_ = nullptr;
        }
        return *this;
    }
    
    // 禁用拷贝构造和拷贝赋值
    SSLContext(const SSLContext&) = delete;
    SSLContext& operator=(const SSLContext&) = delete;
    
    // 获取原始SSL_CTX指针
    SSL_CTX* get() const { return ctx_; }
    
    // 检查是否有效
    bool is_valid() const { return ctx_ != nullptr; }
    
    // 释放所有权
    SSL_CTX* release() {
        SSL_CTX* ctx = ctx_;
        ctx_ = nullptr;
        return ctx;
    }
    
    // 重置为新的SSL_CTX
    void reset(SSL_CTX* ctx = nullptr) {
        if (ctx_) {
            SSL_CTX_free(ctx_);
        }
        ctx_ = ctx;
    }
    
    // 创建新的SSL_CTX
    static SSLContext create(const SSL_METHOD* method) {
        return SSLContext(SSL_CTX_new(method));
    }

private:
    SSL_CTX* ctx_;
};

// SSL的RAII包装器
class SSLSocket {
public:
    // 构造函数
    SSLSocket() : ssl_(nullptr) {}
    
    // 从原始SSL指针构造
    explicit SSLSocket(SSL* ssl) : ssl_(ssl) {}
    
    // 析构函数，自动释放SSL
    ~SSLSocket() {
        if (ssl_) {
            SSL_free(ssl_);
        }
    }
    
    // 移动构造函数
    SSLSocket(SSLSocket&& other) noexcept : ssl_(other.ssl_) {
        other.ssl_ = nullptr;
    }
    
    // 移动赋值操作符
    SSLSocket& operator=(SSLSocket&& other) noexcept {
        if (this != &other) {
            if (ssl_) {
                SSL_free(ssl_);
            }
            ssl_ = other.ssl_;
            other.ssl_ = nullptr;
        }
        return *this;
    }
    
    // 禁用拷贝构造和拷贝赋值
    SSLSocket(const SSLSocket&) = delete;
    SSLSocket& operator=(const SSLSocket&) = delete;
    
    // 获取原始SSL指针
    SSL* get() const { return ssl_; }
    
    // 检查是否有效
    bool is_valid() const { return ssl_ != nullptr; }
    
    // 释放所有权
    SSL* release() {
        SSL* ssl = ssl_;
        ssl_ = nullptr;
        return ssl;
    }
    
    // 重置为新的SSL
    void reset(SSL* ssl = nullptr) {
        if (ssl_) {
            SSL_free(ssl_);
        }
        ssl_ = ssl;
    }
    
    // 创建新的SSL
    static SSLSocket create(SSL_CTX* ctx) {
        return SSLSocket(SSL_new(ctx));
    }
    
    // 执行SSL关闭
    void shutdown() {
        if (ssl_) {
            SSL_shutdown(ssl_);
        }
    }

private:
    SSL* ssl_;
};

#else // 非Linux平台的空实现

class SSLContext {
public:
    SSLContext() = default;
    ~SSLContext() = default;
    
    bool is_valid() const { return false; }
    
    // 空实现
    static SSLContext create(const void* method) {
        (void)method;
        return SSLContext();
    }
};

class SSLSocket {
public:
    SSLSocket() = default;
    ~SSLSocket() = default;
    
    bool is_valid() const { return false; }
    
    // 空实现
    static SSLSocket create(void* ctx) {
        (void)ctx;
        return SSLSocket();
    }
    
    void shutdown() {}
};

#endif // __linux__

} // namespace utils
} // namespace sqlcc

#endif // SQLCC_SSL_WRAPPER_H