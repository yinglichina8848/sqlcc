#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <filesystem>
#include <exception.h>
#include <buffer_pool.h>
#include <config_manager.h>
#include <disk_manager.h>
#include <logger.h>

namespace sqlcc {
namespace test {

// 死锁修复测试类
// Why: 需要验证之前的死锁修复是否真正解决了问题
// What: 创建并发测试场景，模拟之前可能产生死锁的情况
// How: 同时进行配置变更和页面访问操作
class DeadlockFixTest {
public:
    // 构造函数，初始化测试环境
    // Why: 需要创建测试所需的组件
    // What: 初始化磁盘管理器、配置管理器和缓冲池
    // How: 创建必要的对象并设置测试参数
    DeadlockFixTest() : test_running_(true), deadlock_detected_(false) {
        // 设置测试数据库文件路径
        test_db_path_ = "./tests/test_deadlock_fix.db";
        
        // 获取配置管理器单例实例
        config_manager_ = &ConfigManager::GetInstance();
        
        // 注册配置变更回调
        // Note: value参数在当前回调中未使用，使用[[maybe_unused]]标记避免编译警告
        config_manager_->RegisterChangeCallback("buffer_pool.pool_size",
            [this](const std::string& key, [[maybe_unused]] const ConfigValue& value) {
                std::cout << "配置变更回调: " << key << std::endl;
            });
        
        // 创建磁盘管理器
        disk_manager_ = std::make_unique<DiskManager>(test_db_path_, *config_manager_);
        
        // 创建缓冲池，初始大小为10
        buffer_pool_ = std::make_unique<BufferPool>(disk_manager_.get(), 10, *config_manager_);
        
        std::cout << "死锁修复测试环境初始化完成" << std::endl;
    }

    // 析构函数，清理测试环境
    // Why: 需要清理测试产生的文件和对象
    // What: 删除测试数据库文件，销毁测试对象
    // How: 关闭数据库连接，删除测试文件
    ~DeadlockFixTest() {
        test_running_ = false;
        
        // 等待所有线程结束
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 清理测试数据库文件
        try {
            std::filesystem::remove(test_db_path_);
        } catch (...) {
            // 忽略删除文件时的异常
        }
        
        std::cout << "死锁修复测试环境清理完成" << std::endl;
    }

    // 运行死锁修复测试
    // Why: 执行完整的死锁修复验证测试
    // What: 创建多个并发操作，检测是否还会发生死锁
    // How: 同时进行配置变更和页面访问操作
    bool RunDeadlockFixTest() {
        std::cout << "开始死锁修复测试..." << std::endl;
        
        const int num_threads = 4;
        const int operations_per_thread = 50;
        
        std::vector<std::thread> threads;
        
        // 创建配置变更线程
        threads.emplace_back([this, num_threads, operations_per_thread]() {
            for (int i = 0; i < num_threads; ++i) {
                if (!test_running_) break;
                
                try {
                    // 模拟配置变更
                    std::string config_key = "buffer_pool.pool_size";
                    size_t new_pool_size = 10 + (i % 5);
                    
                    // 直接设置配置值，这会触发回调
                    config_manager_->SetValue(config_key, static_cast<int>(new_pool_size));
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    
                } catch (const std::exception& e) {
                    std::cerr << "配置变更线程异常: " << e.what() << std::endl;
                    deadlock_detected_ = true;
                }
            }
        });
        
        // 创建页面访问线程
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([this, i, operations_per_thread]() {
                for (int j = 0; j < operations_per_thread; ++j) {
                    if (!test_running_) break;
                    
                    try {
                        int32_t page_id = (j % 20) + 1;  // 使用页面ID 1-20
                        
                        // 获取页面
                        Page* page = buffer_pool_->FetchPage(page_id);
                        if (page != nullptr) {
                            // 模拟页面访问
                            std::this_thread::sleep_for(std::chrono::microseconds(100));
                            
                            // 取消固定页面
                            buffer_pool_->UnpinPage(page_id, false);
                        }
                        
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        
                    } catch (const std::exception& e) {
                        std::cerr << "页面访问线程异常 (线程" << i << "): " << e.what() << std::endl;
                        deadlock_detected_ = true;
                    }
                }
            });
        }
        
        // 创建预取线程
        for (int i = 0; i < num_threads / 2; ++i) {
            threads.emplace_back([this, i]() {
                for (int j = 0; j < 20; ++j) {
                    if (!test_running_) break;
                    
                    try {
                        int32_t page_id = (j % 15) + 1;
                        
                        // 执行预取操作
                        buffer_pool_->PrefetchPage(page_id);
                        
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        
                    } catch (const std::exception& e) {
                        std::cerr << "预取线程异常 (线程" << i << "): " << e.what() << std::endl;
                        deadlock_detected_ = true;
                    }
                }
            });
        }
        
        // 等待所有线程完成（最多等待30秒）
        auto start_time = std::chrono::steady_clock::now();
        const auto max_wait_time = std::chrono::seconds(30);
        
        for (auto& thread : threads) {
            if (thread.joinable()) {
                auto elapsed = std::chrono::steady_clock::now() - start_time;
                if (elapsed < max_wait_time) {
                    thread.join();
                } else {
                    std::cerr << "线程执行超时，检测到可能的死锁!" << std::endl;
                    test_running_ = false;
                    deadlock_detected_ = true;
                    thread.detach();  // 分离超时线程
                }
            }
        }
        
        // 检查测试结果
        if (deadlock_detected_) {
            std::cout << "❌ 测试失败: 检测到死锁或异常" << std::endl;
            return false;
        } else {
            std::cout << "✅ 测试通过: 未检测到死锁" << std::endl;
            return true;
        }
    }

private:
    // 测试运行标志
    // Why: 控制测试线程的运行状态
    // What: atomic_bool类型，支持原子操作
    // How: 用于优雅地停止测试线程
    std::atomic<bool> test_running_;
    
    // 死锁检测标志
    // Why: 检测是否发生了死锁或其他异常
    // What: atomic_bool类型，用于线程安全的标志位
    // How: 当检测到异常时设置为true
    std::atomic<bool> deadlock_detected_;
    
    // 测试数据库路径
    // Why: 指定测试用数据库文件的位置
    // What: std::string类型，存储文件路径
    // How: 创建和删除测试数据库文件
    std::string test_db_path_;
    
    /**
     * @brief 配置管理器指针
     * Why: 管理配置变更和回调
     * What: ConfigManager对象指针
     * How: 注册回调函数并触发配置变更
     */
    ConfigManager* config_manager_;
    
    /**
     * @brief 磁盘管理器指针
     * Why: 处理磁盘I/O操作
     * What: DiskManager对象指针
     * How: 管理数据库文件的读写操作
     */
    std::unique_ptr<DiskManager> disk_manager_;
    
    /**
     * @brief 缓冲池指针
     * Why: 测试缓冲池的并发操作
     * What: BufferPool对象指针
     * How: 执行页面获取、预取等操作
     */
    std::unique_ptr<BufferPool> buffer_pool_;
};

} // namespace test
} // namespace sqlcc

int main() {
    std::cout << "=== SQLCC 死锁修复测试 ===" << std::endl;
    std::cout << "测试目的: 验证BufferPool死锁修复是否有效" << std::endl;
    std::cout << std::endl;
    
    try {
        // 创建死锁修复测试实例
        sqlcc::test::DeadlockFixTest test;
        
        // 运行测试
        bool test_passed = test.RunDeadlockFixTest();
        
        std::cout << std::endl;
        if (test_passed) {
            std::cout << "🎉 死锁修复测试成功!" << std::endl;
            std::cout << "BufferPool的锁顺序和回调机制修复有效。" << std::endl;
            return 0;
        } else {
            std::cout << "💥 死锁修复测试失败!" << std::endl;
            std::cout << "仍存在死锁问题，需要进一步调查和修复。" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "测试执行异常: " << e.what() << std::endl;
        return 1;
    }
}