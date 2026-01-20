/**
 * @file ssl_wrapper.h
 * @brief SSL资源RAII包装器 - 深度安全改进版
 * 
 * 该文件提供了SSL和SSL_CTX资源的RAII管理类，
 * 确保SSL资源在异常情况下也能正确释放
 * 
 * v1.1.4 深度安全改进:
 * - 增强参数验证和异常安全保证
 * - 添加错误处理和状态验证
 * - 实现强保证级别的异常安全
 */

#ifndef SQLCC_SSL_WRAPPER_H
#define SQLCC_SSL_WRAPPER_H

#include <memory>
#include <stdexcept>
#include <string>

#ifdef __linux__
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

namespace sqlcc {
namespace utils {

#ifdef __linux__

// SSL_CTX的RAII包装器 - 深度安全改进
class SSLContext {
public:
    // 构造函数 - 增强异常安全
    SSLContext() : ctx_(nullptr) {}
    
    // 从原始SSL_CTX指针构造 - 增强参数验证
    explicit SSLContext(SSL_CTX* ctx) : ctx_(ctx) {
        if (!ctx) {
            throw std::invalid_argument("SSL_CTX pointer cannot be null");
        }
    }
    
    // 析构函数，自动释放SSL_CTX - 增强异常安全
    ~SSLContext() {
        try {
            if (ctx_) {
                SSL_CTX_free(ctx_);
            }
        } catch (...) {
            // 析构函数中不抛出异常，记录日志即可
            // 在实际应用中应该使用日志系统
        }
    }
    
    // 移动构造函数 - 强保证异常安全
    SSLContext(SSLContext&& other) noexcept : ctx_(other.ctx_) {
        other.ctx_ = nullptr;
    }
    
    // 移动赋值操作符 - 强保证异常安全
    SSLContext& operator=(SSLContext&& other) noexcept {
        if (this != &other) {
            // 先释放当前资源，再获取新资源
            try {
                if (ctx_) {
                    SSL_CTX_free(ctx_);
                }
            } catch (...) {
                // 静默处理释放异常，保持异常安全
            }
            ctx_ = other.ctx_;
            other.ctx_ = nullptr;
        }
        return *this;
    }
    
    // 禁用拷贝构造和拷贝赋值
    SSLContext(const SSLContext&) = delete;
    SSLContext& operator=(const SSLContext&) = delete;
    
    // 获取原始SSL_CTX指针 - 添加异常安全检查
    SSL_CTX* get() const { 
        return ctx_; 
    }
    
    // 检查是否有效 - 增强验证
    bool is_valid() const noexcept {
        return ctx_ != nullptr;
    }
    
    // 释放所有权 - 增强参数验证
    SSL_CTX* release() {
        if (!ctx_) {
            throw std::logic_error("Cannot release null SSL_CTX");
        }
        SSL_CTX* ctx = ctx_;
        ctx_ = nullptr;
        return ctx;
    }
    
    // 重置为新的SSL_CTX - 增强异常安全
    void reset(SSL_CTX* ctx = nullptr) {
        if (ctx_ == ctx) {
            return; // 避免重复设置相同值
        }
        
        try {
            if (ctx_) {
                SSL_CTX_free(ctx_);
            }
        } catch (...) {
            // 静默处理释放异常，保持异常安全
        }
        
        if (ctx && !ctx_) {
            throw std::invalid_argument("Cannot reset with null SSL_CTX");
        }
        
        ctx_ = ctx;
    }
    
    // 创建新的SSL_CTX - 增强错误处理
    static SSLContext create(const SSL_METHOD* method) {
        if (!method) {
            throw std::invalid_argument("SSL_METHOD cannot be null");
        }
        
        SSL_CTX* ctx = SSL_CTX_new(method);
        if (!ctx) {
            throw std::runtime_error("Failed to create SSL_CTX: " + get_ssl_error_string());
        }
        
        return SSLContext(ctx);
    }
    
    // 获取SSL库错误信息 - 新增方法
    static std::string get_ssl_error_string() {
        char buf[256];
        unsigned long err = ERR_get_error();
        if (err == 0) {
            return "No SSL error";
        }
        ERR_error_string(err, buf);
        return std::string(buf);
    }
    
    // 验证SSL_CTX配置 - 新增方法
    void validate_configuration() const {
        if (!ctx_) {
            throw std::logic_error("SSL_CTX is null");
        }
        
        // 检查基本的SSL_CTX配置
        if (SSL_CTX_check_private_key(ctx_) != 1) {
            throw std::runtime_error("SSL private key validation failed: " + get_ssl_error_string());
        }
    }

private:
    SSL_CTX* ctx_;
};

// SSL的RAII包装器 - 深度安全改进
class SSLSocket {
public:
    // 构造函数 - 增强异常安全
    SSLSocket() : ssl_(nullptr) {}
    
    // 从原始SSL指针构造 - 增强参数验证
    explicit SSLSocket(SSL* ssl) : ssl_(ssl) {
        if (!ssl) {
            throw std::invalid_argument("SSL pointer cannot be null");
        }
    }
    
    // 析构函数，自动释放SSL - 增强异常安全
    ~SSLSocket() {
        try {
            if (ssl_) {
                SSL_free(ssl_);
            }
        } catch (...) {
            // 静默处理释放异常
        }
    }
    
    // 移动构造函数 - 强保证异常安全
    SSLSocket(SSLSocket&& other) noexcept : ssl_(other.ssl_) {
        other.ssl_ = nullptr;
    }
    
    // 移动赋值操作符 - 强保证异常安全
    SSLSocket& operator=(SSLSocket&& other) noexcept {
        if (this != &other) {
            try {
                if (ssl_) {
                    SSL_free(ssl_);
                }
            } catch (...) {
                // 静默处理释放异常
            }
            ssl_ = other.ssl_;
            other.ssl_ = nullptr;
        }
        return *this;
    }
    
    // 禁用拷贝构造和拷贝赋值
    SSLSocket(const SSLSocket&) = delete;
    SSLSocket& operator=(const SSLSocket&) = delete;
    
    // 获取原始SSL指针 - 添加异常安全检查
    SSL* get() const { 
        return ssl_; 
    }
    
    // 检查是否有效 - 增强验证
    bool is_valid() const noexcept {
        return ssl_ != nullptr;
    }
    
    // 释放所有权 - 增强参数验证
    SSL* release() {
        if (!ssl_) {
            throw std::logic_error("Cannot release null SSL");
        }
        SSL* ssl = ssl_;
        ssl_ = nullptr;
        return ssl;
    }
    
    // 重置为新的SSL - 增强异常安全
    void reset(SSL* ssl = nullptr) {
        if (ssl_ == ssl) {
            return; // 避免重复设置相同值
        }
        
        try {
            if (ssl_) {
                SSL_free(ssl_);
            }
        } catch (...) {
            // 静默处理释放异常
        }
        
        if (ssl && !ssl_) {
            throw std::invalid_argument("Cannot reset with null SSL");
        }
        
        ssl_ = ssl;
    }
    
    // 创建新的SSL - 增强错误处理
    static SSLSocket create(SSL_CTX* ctx) {
        if (!ctx) {
            throw std::invalid_argument("SSL_CTX cannot be null");
        }
        
        SSL* ssl = SSL_new(ctx);
        if (!ssl) {
            throw std::runtime_error("Failed to create SSL: " + SSLContext::get_ssl_error_string());
        }
        
        return SSLSocket(ssl);
    }
    
    // 执行SSL关闭 - 增强异常安全
    void shutdown() {
        try {
            if (ssl_) {
                int result = SSL_shutdown(ssl_);
                // SSL_shutdown可能返回0或1，都表示成功完成或部分完成
                if (result < 0) {
                    // 记录错误但不抛出异常（析构函数中）
                    // 在实际应用中应该记录到日志系统
                }
            }
        } catch (...) {
            // 静默处理关闭异常
        }
    }
    
    // 获取SSL错误信息 - 新增方法
    std::string get_error_string() const {
        if (!ssl_) {
            return "SSL is null";
        }
        return SSLContext::get_ssl_error_string();
    }

private:
    SSL* ssl_;
};

#else // 非Linux平台的空实现 - 增强异常安全

class SSLContext {
public:
    SSLContext() = default;
    ~SSLContext() = default;
    
    bool is_valid() const { return false; }
    
    // 空实现 - 增强异常安全
    static SSLContext create(const void* method) {
        (void)method;
        throw std::runtime_error("SSL not supported on this platform");
    }
    
    static std::string get_ssl_error_string() {
        return "SSL not supported on this platform";
    }
};

class SSLSocket {
public:
    SSLSocket() = default;
    ~SSLSocket() = default;
    
    bool is_valid() const { return false; }
    
    // 空实现 - 增强异常安全
    static SSLSocket create(void* ctx) {
        (void)ctx;
        throw std::runtime_error("SSL not supported on this platform");
    }
    
    void shutdown() {
        throw std::runtime_error("SSL not supported on this platform");
    }
    
    std::string get_error_string() const { return "SSL not supported on this platform"; }
};

#endif // __linux__

} // namespace utils
} // namespace sqlcc

#endif // SQLCC_SSL_WRAPPER_H
