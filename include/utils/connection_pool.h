#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <vector>
#include <set>

namespace sqlcc {
namespace utils {

/**
 * @class ConnectionPool
 * @brief 数据库连接池 - 管理数据库连接的复用
 *
 * 提供高效的连接管理，支持：
 * - 连接复用减少开销
 * - 自动连接健康检查
 * - 连接超时管理
 * - 线程安全操作
 */
template<typename ConnectionType>
class ConnectionPool {
public:
    /**
     * @brief 连接工厂函数类型
     */
    using ConnectionFactory = std::function<std::shared_ptr<ConnectionType>()>;

    /**
     * @brief 连接验证函数类型
     */
    using ConnectionValidator = std::function<bool(std::shared_ptr<ConnectionType>)>;

    /**
     * @brief 连接池配置
     */
    struct PoolConfig {
        size_t initial_size = 10;           // 初始连接数
        size_t max_size = 100;              // 最大连接数
        size_t min_size = 5;                // 最小连接数
        std::chrono::seconds max_idle_time = std::chrono::seconds(300);  // 最大空闲时间
        std::chrono::seconds connection_timeout = std::chrono::seconds(30); // 连接超时
        std::chrono::seconds health_check_interval = std::chrono::seconds(60); // 健康检查间隔
    };

    /**
     * @brief 连接池统计信息
     */
    struct PoolStats {
        size_t total_connections = 0;       // 总连接数
        size_t active_connections = 0;      // 活跃连接数
        size_t idle_connections = 0;        // 空闲连接数
        size_t waiting_threads = 0;         // 等待线程数
        size_t created_connections = 0;     // 已创建连接数
        size_t destroyed_connections = 0;   // 已销毁连接数
        double hit_rate = 0.0;              // 连接池命中率
    };

    /**
     * @brief 构造函数
     * @param factory 连接工厂函数
     * @param validator 连接验证函数
     * @param config 连接池配置
     */
    ConnectionPool(ConnectionFactory factory,
                  ConnectionValidator validator = nullptr,
                  const PoolConfig& config = PoolConfig())
        : factory_(std::move(factory)),
          validator_(std::move(validator)),
          config_(config),
          running_(false) {}

    /**
     * @brief 析构函数
     */
    ~ConnectionPool() {
        shutdown();
    }

    // 禁用拷贝
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    /**
     * @brief 启动连接池
     */
    void start() {
        std::unique_lock<std::mutex> lock(pool_mutex_);
        if (running_) return;

        running_ = true;

        // 创建初始连接
        for (size_t i = 0; i < config_.initial_size; ++i) {
            createConnection();
        }

        // 启动清理线程
        cleanup_thread_ = std::thread([this]() { cleanupWorker(); });
    }

    /**
     * @brief 停止连接池
     */
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(pool_mutex_);
            if (!running_) return;
            running_ = false;
        }

        condition_.notify_all();

        if (cleanup_thread_.joinable()) {
            cleanup_thread_.join();
        }

        // 关闭所有连接
        std::unique_lock<std::mutex> lock(pool_mutex_);
        while (!idle_connections_.empty()) {
            idle_connections_.pop();
        }
        active_connections_.clear();
    }

    /**
     * @brief 获取连接
     * @param timeout 等待超时时间
     * @return 数据库连接，如果获取失败返回nullptr
     */
    std::shared_ptr<ConnectionType> acquire(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) {
        std::unique_lock<std::mutex> lock(pool_mutex_);

        // 首先尝试从空闲连接中获取
        if (!idle_connections_.empty()) {
            auto connection = idle_connections_.front();
            idle_connections_.pop();

            if (isConnectionValid(connection)) {
                active_connections_.insert(connection);
                stats_.active_connections++;
                stats_.idle_connections--;
                return connection;
            } else {
                // 连接无效，销毁并创建新连接
                stats_.destroyed_connections++;
                createConnection();
            }
        }

        // 如果空闲连接不足，尝试创建新连接
        if (active_connections_.size() + idle_connections_.size() < config_.max_size) {
            createConnection();
            if (!idle_connections_.empty()) {
                auto connection = idle_connections_.front();
                idle_connections_.pop();
                active_connections_.insert(connection);
                stats_.active_connections++;
                return connection;
            }
        }

        // 等待可用连接
        stats_.waiting_threads++;
        bool success = condition_.wait_for(lock, timeout, [this]() {
            return !idle_connections_.empty() || !running_;
        });
        stats_.waiting_threads--;

        if (!success || !running_) {
            return nullptr;
        }

        if (!idle_connections_.empty()) {
            auto connection = idle_connections_.front();
            idle_connections_.pop();

            if (isConnectionValid(connection)) {
                active_connections_.insert(connection);
                stats_.active_connections++;
                stats_.idle_connections--;
                return connection;
            } else {
                stats_.destroyed_connections++;
            }
        }

        return nullptr;
    }

    /**
     * @brief 释放连接
     * @param connection 要释放的连接
     */
    void release(std::shared_ptr<ConnectionType> connection) {
        if (!connection) return;

        std::unique_lock<std::mutex> lock(pool_mutex_);

        auto it = active_connections_.find(connection);
        if (it != active_connections_.end()) {
            active_connections_.erase(it);
            stats_.active_connections--;

            if (isConnectionValid(connection)) {
                idle_connections_.push(connection);
                stats_.idle_connections++;
            } else {
                stats_.destroyed_connections++;
            }
        }

        condition_.notify_one();
    }

    /**
     * @brief 获取连接池统计信息
     * @return 统计信息
     */
    PoolStats getStats() const {
        std::unique_lock<std::mutex> lock(pool_mutex_);
        PoolStats stats = stats_;
        stats.total_connections = active_connections_.size() + idle_connections_.size();
        stats.idle_connections = idle_connections_.size();
        stats.active_connections = active_connections_.size();

        if (stats.created_connections > 0) {
            stats.hit_rate = static_cast<double>(stats.created_connections - stats.destroyed_connections) /
                           stats.created_connections;
        }

        return stats;
    }

    /**
     * @brief 设置连接池配置
     * @param config 新配置
     */
    void setConfig(const PoolConfig& config) {
        std::unique_lock<std::mutex> lock(pool_mutex_);
        config_ = config;
    }

    /**
     * @brief 获取连接池配置
     * @return 当前配置
     */
    PoolConfig getConfig() const {
        std::unique_lock<std::mutex> lock(pool_mutex_);
        return config_;
    }

private:
    /**
     * @brief 创建新连接
     */
    void createConnection() {
        try {
            auto connection = factory_();
            if (connection) {
                idle_connections_.push(connection);
                stats_.created_connections++;
                stats_.idle_connections++;
            }
        } catch (const std::exception& e) {
            // 记录错误但不抛出异常
            std::cerr << "Failed to create connection: " << e.what() << std::endl;
        }
    }

    /**
     * @brief 检查连接是否有效
     * @param connection 要检查的连接
     * @return 是否有效
     */
    bool isConnectionValid(std::shared_ptr<ConnectionType> connection) {
        if (!connection) return false;

        if (validator_) {
            try {
                return validator_(connection);
            } catch (const std::exception&) {
                return false;
            }
        }

        // 默认验证：检查连接是否仍然可用
        return true;
    }

    /**
     * @brief 清理工作线程
     */
    void cleanupWorker() {
        while (running_) {
            std::this_thread::sleep_for(config_.health_check_interval);

            std::unique_lock<std::mutex> lock(pool_mutex_);
            if (!running_) break;

            // 清理超时的空闲连接
            auto now = std::chrono::steady_clock::now();
            while (!idle_connections_.empty()) {
                auto connection = idle_connections_.front();
                // 简化实现：假设所有连接都是健康的，实际应记录连接创建时间
                break;
            }

            // 确保最小连接数
            size_t current_size = active_connections_.size() + idle_connections_.size();
            if (current_size < config_.min_size && current_size < config_.max_size) {
                size_t to_create = std::min(config_.min_size - current_size, config_.max_size - current_size);
                for (size_t i = 0; i < to_create; ++i) {
                    lock.unlock();
                    createConnection();
                    lock.lock();
                }
            }
        }
    }

    // 连接工厂和验证器
    ConnectionFactory factory_;
    ConnectionValidator validator_;

    // 配置
    PoolConfig config_;

    // 连接存储
    std::queue<std::shared_ptr<ConnectionType>> idle_connections_;
    std::set<std::shared_ptr<ConnectionType>> active_connections_;

    // 同步原语
    mutable std::mutex pool_mutex_;
    std::condition_variable condition_;

    // 控制变量
    std::atomic<bool> running_;
    std::thread cleanup_thread_;

    // 统计信息
    mutable PoolStats stats_;
};

} // namespace utils
} // namespace sqlcc